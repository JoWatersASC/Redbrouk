#include <charconv>
#include <cstdlib>
#include <memory>
#include <signal.h>
#include <string>

#include "src/kvobj.h"
#include "src/network.h"

#include "src/io.h"
#include "src/kvt_map.h"
#include "src/kvt_string.h"
#include "src/kvt_tset.h"

namespace redbrouk
{

using utils::fmt;
// Hacky way to reclaim sockets faster(automatically) after signal received.
// execl to trigger CLO_EXEC
void sigint_handler(int sig_num) {
	std::println("\nCaught SIGINT {}! Performing cleanup...", sig_num);
	std::println("Exiting gracefully.\n");
	execl("/bin/ls", NULL);
    exit(0);
}

void io_context::init(uint16_t _port) {
	socket_t listen_fd = make_listener(_port);
	if(listen_fd == -1) {
		return;
	}
	if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("Error setting up signal handler");
        return;
    }
	if (signal(SIGSEGV, sigint_handler) == SIG_ERR) {
        perror("Error setting up signal handler");
        return;
    }
	if (signal(SIGABRT, sigint_handler) == SIG_ERR) {
        perror("Error setting up signal handler");
        return;
    }

	pfds.resize(1);
	pfds.reserve(8);
	pfds[0] = { .fd = listen_fd, .events = POLLIN, .revents = 0 };
}
void io_context::main_loop() {
	std::vector<Conn *> connections;

	struct pollfd listener = pfds[0];
	while(true) {
		if(!running)
			return;

		pfds.clear();
		pfds.push_back(listener);

		for(Conn *conn : connections) {
			if(!conn)
				continue;

			struct pollfd pfd{ conn->get_socket(), POLLERR, 0 };

			if((bool)(conn->state & ConnState::RECVING))
				pfd.events |= POLLIN;
			if((bool)(conn->state & ConnState::SENDING))
				pfd.events |= POLLOUT;

			pfds.push_back(pfd);
		}

		ssize_t rv = poll(pfds.data(), (nfds_t)pfds.size(), -1);
		if(rv < 0) {
			if(errno == EINTR)
				continue;

			std::println("[ERROR] main_loop {}", strerror(errno));
			exit(1);
		}

		if(pfds[0].revents) {
			if(Conn *conn = Accept(listener.fd)) {
				const socket_t fd = conn->get_socket();
				if(connections.size() <= (size_t)fd) {
					connections.resize(fd + 1);
				}

				assert(!connections[fd]);
				connections[fd] = conn;
				conn->in_buff = recv_buf;
				conn->ot_buff = send_buf;
			}
		}

		for(size_t i = 1; i < pfds.size(); i++) {
			uint32_t ready = pfds[i].revents;

			if(ready == 0)
				continue;

			Conn *conn = connections[pfds[i].fd];
			if (ready & POLLIN) {
				assert(conn->state & ConnState::RECVING);
				handle_read(conn);  // application logic
			}
			if (ready & POLLOUT) {
				assert(conn->state & ConnState::SENDING);
				handle_write(conn); // application logic
			}

			if((ready & POLLERR) || (bool)(conn->state & ConnState::CLOSED)) {
				conn->Close();
				connections[conn->get_socket()] = nullptr;
				delete conn;
			}
		}
	}
}

static void handle_read(Conn *conn) {
	std::byte buf[64 * 1024];
	ssize_t rv = conn->recv(buf, sizeof(buf));

	if (rv < 0 && errno == EAGAIN)
		return;

	if (rv < 0) {
		conn->state = ConnState::CLOSED | ConnState::ERR;
		std::println("[ERROR] handle_read: {}", strerror(errno));
		return;
	}

	// handle EOF
	if (rv == 0) {
		if (conn->in_start == conn->in_end) {
			std::println("client closed");
		} else {
			std::println("unexpected EOF");
		}

		conn->state = ConnState::CLOSED;
		return; // want close
	}

	memcpy(conn->in_data(), buf, (size_t)rv);

	while (try_request(conn));

	if (conn->ot_size() > 0) {    // has a response
		conn->state &= ~ConnState::RECVING;
		conn->state |=  ConnState::SENDING;

		return handle_write(conn);
	}
}
static void handle_write(Conn *conn) {
	ssize_t rv = conn->send(conn->ot_data(), conn->ot_size());

	if (rv < 0 && errno == EAGAIN)
		return;

	if (rv < 0) {
		conn->state = ConnState::CLOSED | ConnState::ERR;
		std::println("[ERROR] handle_read: {}", strerror(errno));
		return;
	}

	if (conn->ot_start == conn->ot_end) {   // all data written
		conn->state &= ~ConnState::SENDING;
		conn->state |=  ConnState::RECVING;
	}
}

using std::vector;

static struct {
	iHMap kvs; // key-value store
	KVObj data[1024 * 16];
	size_t data_idx;
} db;

inline KVObj *emplace_kvobj(std::string_view _key, KVTYPE _type) {
	KVObj *obj = new (&db.data[db.data_idx++]) KVObj(_type);
	obj->set_key(std::string(_key));
	ihs_insert(&db.kvs.table, obj->hook());
	return obj;
}

namespace {
	bool lookup_eq(const iHNode *a, const iHNode *b) {
		KVObj &obj           = get_kvobj_v((iHNode *)a);
		LookupDummy *lookup  = utils::container_of((iHNode *)b, &LookupDummy::hook);
	
		return obj.get_key() == lookup->key;
	}
}

static void do_request(std::vector<sview> &cmd, Response &out) {
	using req_func = void(std::vector<sview>&, Response&);
	req_func get_val, set_val, del_val,
			do_add_tset, do_range_tset;

	const size_t cmd_len = cmd.size();

	if (cmd_len == 2 && cmd[0] == "get") {
		get_val(cmd, out);
		if(out.status == RES_NX)
			return;

	} else if (cmd_len == 3 && cmd[0] == "set") {
		set_val(cmd, out);
	} else if (cmd_len == 2 && cmd[0] == "del") {
		del_val(cmd, out);
	} else if (cmd_len >= 4 && cmd[0] == "tadd") {
		do_add_tset(cmd, out);
	} else if (cmd_len == 4 && cmd[0] == "trange") {
		do_range_tset(cmd, out);
	} else {
		out.status = RES_ERR;
	}
}

namespace {
	static void make_response(const Response &res, byte *out, off_t &offset) {
		uint32_t rlen = 4 + (uint32_t)res.data.size();

		memcpy(out + offset, (byte *)&rlen, sizeof(rlen));
		offset += sizeof(rlen);
		memcpy(out + offset, (byte *)&res.status, sizeof(res.status));
		offset += sizeof(res.status);
		memcpy(out + offset, (byte *)res.data.data(), res.data.size());
		offset += res.data.size();
	}
}

static int try_request(Conn *conn) {
	if(conn->in_size() < 4)
		return false;

	uint32_t len = 0;
	memcpy(&len, conn->in_data(), sizeof(len));
	if(len > 20000) {
		std::println("too long");
		conn->state = ConnState::CLOSED;

		return false;
	}

	if(len + sizeof(len) > conn->in_size())
		return false;

	const byte *request = conn->in_data() + 4;

	std::vector<sview> cmd;
	if(parse_req(request, len, cmd) < 0) {
		std::println("Bad request");
		conn->state = ConnState::CLOSED;

		return false;
	}

	Response res;
	do_request(cmd, res);
	make_response(res, conn->ot_buff, conn->ot_end);

	conn->in_start += 4 + len;
	return true;
}

static int32_t parse_req(const byte *data, size_t len, std::vector<sview>& out) {
	const byte *end = data + len;
	uint32_t nstr = 0;

	if(end - data < 4)
		return -1;

	memcpy(&nstr, data, sizeof(nstr));
	data += 4;

	if(nstr > 20000)
		return -1;

	while(out.size() < nstr) {
		uint32_t str_len = 0;
		if(end - data < sizeof(str_len))
			return -1;
		memcpy(&str_len, data, sizeof(str_len));
		data += sizeof(str_len);

		if(end - data < str_len)
			return -1;

		out.emplace_back((const char *)data, str_len);
		data += str_len;
	}

	if(data != end)
		return -1;

	return 0;
}

//---------------------------------------------------------------------------------------
// String val type functions
//---------------------------------------------------------------------------------------
void get_val(vector<sview> &cmds, Response &out) { // get command from cmds vector
	LookupDummy dummy{
		.hook = { nullptr, genHash((const byte *)cmds[1].data(), cmds[1].length()) },
		.key  = cmds[1]
	};

	iHNode *node = ihs_find(&db.kvs.table, &dummy.hook, lookup_eq);

	if(!node) {
		out.status = RES_NX;
		return;
	}

	std::string res;
	KVObj &container = get_kvobj_v(node);

	if(container.type() != KVTYPE::STRING) {
		res = "[ERROR: TYPE_MM] Was expecting STRING type";
		out.data.assign(res.begin(), res.end());
	} else {
		res = "[GET] Key: " + std::string(cmds[1]) + " Val: " + (String&)container.val();
	}

	out.data.assign(res.begin(), res.end());
}
void set_val(vector<sview> &cmds, Response &out) {
	LookupDummy dummy{
		.hook = { nullptr, genHash((const byte *)cmds[1].data(), cmds[1].length()) },
		.key  = cmds[1]
	};
	std::string res;
	KVObj *entry;
	iHNode *_hook;

	_hook = ihs_find(&db.kvs.table, &dummy.hook, lookup_eq);
	if(!_hook) {
		entry = emplace_kvobj(dummy.key, KVTYPE::STRING);
		entry->make_val<KVTYPE::STRING>(std::move(cmds[2]));
	}
	else if(get_kvobj_v(_hook).type() != KVTYPE::STRING) {
		res = "[ERROR: TYPE_MM] Was expecting STRING type";
		out.data.assign(res.begin(), res.end());
		out.status = RES_ERR;
		return;
	}

	res = "[SET] Key: " + entry->get_key() + " Val: " + (String&)entry->val();

	out.data.assign(res.begin(), res.end());
	out.status = RES_OK;
}
void del_val(vector<sview> &cmds, Response &out) {
	LookupDummy dummy{
		.hook = { nullptr, genHash((const byte*)cmds[1].data(), cmds[1].length()) },
		.key  = cmds[1]
	};

	iHNode *del_node = ihs_find(&db.kvs.table, &dummy.hook, lookup_eq);
	if(!del_node) {
		out.status = RES_NX;
		return;
	}

	ihs_del(&db.kvs.table, &dummy.hook, lookup_eq);

	std::string res;
	KVObj &container = get_kvobj_v(del_node);

	if(container.type() != KVTYPE::STRING) {
		res = "[ERROR: TYPE_MM] Was expecting STRING type";
		out.status = RES_ERR;
	} else {
		res = "[DEL] Key: " + container.get_key() + " Val: " + (String&)container.val();
	}

	out.data.assign(res.begin(), res.end());
	out.status = RES_OK;
}
//---------------------------------------------------------------------------------------
// TSet valtype functons
//---------------------------------------------------------------------------------------
static const TSet NILTSET;
// find_tset: takes a string_view and returns 1 of 3 possibilities.:
// 1) A pointer to the tset that exsts inside of the global db
// 2) A pointer to NILTSET, meaning no kv object exists for that key yet
// 3) A null pointer, meaning a kv object exists but is NOT of type TSet
TSet *find_tset(std::string_view key) {
	LookupDummy dummy{
		nullptr, genHash((const byte *)key.data(), key.length()),
		key
	};

	iHNode *_hook = ihs_find(&db.kvs.table, &dummy.hook, lookup_eq);
	if(!_hook)
		return (TSet *)&NILTSET;
	
	KVObj &container = get_kvobj_v(_hook);
	if(container.type() != KVTYPE::TSET)
		return nullptr;

	return std::addressof((TSet&)container.val());
}
void do_range_tset(vector<sview> &cmds, Response &out) {
	std::string res = "[ERROR: TYPE_MM] Was expecting TSET type";
	TSet *tset = find_tset(cmds[1]);

	if(!tset) {
		out.data.assign(res.begin(), res.end());
		out.status = RES_ERR;
		return;
	}
	if(tset == &NILTSET) {
		res = "[NULL] No TSet object with key " + std::string(cmds[1]);
		out.data.assign(res.begin(), res.end());
		out.status = RES_NX;
		return;
	}
	res.clear();

	ssize_t begin, end, tsize = ts_size(tset);
	std::from_chars(cmds[2].data(), cmds[2].data() + cmds[2].size(), begin);
	std::from_chars(cmds[3].data(), cmds[3].data() + cmds[3].size(), end);

	if(end < 0)
		end = tsize + end;
	if(end >= tsize || end < 0) {
		res = fmt("[ERROR: RANGE_OOB]: Specified range is outside the bounds 0 - {}", tsize - 1);
		out.data.assign(res.begin(), res.end());
		out.status = RES_ERR;
		return;
	}

	res.append("[TRANGE] " + std::to_string(end - begin + 1) + "\n");
	TSTNode *node = ts_at(tset, begin);

	TSTNode *first = node; // to keep track of the first node in the same value list
	for(int i = begin; i <= end && node; i++) {
		res.append( fmt("{}) {}\n", i - begin, node->name) );
		node = ts_walk(node, 1, first);
	}
	out.data.assign(res.begin(), res.end());
	out.status = RES_OK;
}

void do_add_tset(vector<sview> &cmds, Response &out) {
	std::string res = "[ERROR: TYPE_MM] Was expecting TSET type";
	const size_t size = cmds.size() - 1;
	TSet *tset = find_tset(cmds[1]);

	if(!tset) {
		out.data.assign(res.begin(), res.end());
		out.status = RES_ERR;
		return;
	}
	if(tset == &NILTSET) { // No kv object with this key found, create new object of type tset w/ this key
		tset = (TSet*)emplace_kvobj(cmds[1], KVTYPE::TSET)->val_p();
	}

	double _score;
	size_t inserted = 0, updated = 0;
	for(size_t i = 2; i < size; i++) {
		_score = strtod(cmds[i + 1].data(), nullptr);

		if( TSTNode *found = ts_find(tset, cmds[i]) ) {
			ts_update(tset, found, _score);
			updated++;
		} else {
			ts_insertn(tset, std::string(cmds[i]), _score);
			inserted++;
		}
		i++;
	}

	res = fmt("[TADD] Inserted: {}, Updated: {}", inserted, updated);
	out.data.assign(res.begin(), res.end());
	out.status = RES_OK;
}

} // namespace redbrouk
