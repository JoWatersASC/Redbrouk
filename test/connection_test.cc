#include <iostream>
#include <print>
#include <vector>

using std::vector;
using std::print;
using std::println;

#include <cstring>
#include <poll.h>


#include "connection.h"

using namespace redbrouk;
// using std::println;

constexpr uint16_t PORT = 16000;
constexpr uint16_t MAX_CLIENTS = 10;

struct pollfd fds[10];
int nfds = 1;
void __attribute__((constructor)) init_fds() {
	memset(fds, 0, sizeof(fds));
}

vector<Conn *> connections;
alignas(64) char buffer[1024];

int main() {
	socket_t client_fd, listen_fd = make_listener(PORT);

	print("Server listening on port {}\n", 16000);

	vector<struct pollfd> pfds;

	auto get_pe = [&](Conn& c) -> short {
		short out = POLLERR;

		if((int)(c.state & ConnState::SENDING))
			out |= POLLOUT;
		if((int)(c.state & ConnState::RECVING))
			out |= POLLIN;

		return out;
	};

	while(true)
	{
		pfds.clear();
		pfds.push_back({ .fd = listen_fd, .events = POLLERR | POLLIN, .revents = 0 });

		for(auto c : connections) {
			if(!c)
				continue;

			pfds.push_back({ .fd = c->getSocket(), .events = get_pe(*c), .revents = 0 });
			println("Connection {}: {}", c->getSocket(), pfds.back().events);

		}
		assert(pfds.size() > 0 && "listener pfd pushed to vector");

		int ready = poll(pfds.data(), (nfds_t)pfds.size(), -1);

		if(ready < 0) {
			if(errno == EINTR) {
				std::println("Waiting...\r");
				continue;
			}

			std::println("Error on poll readying");
			println("[ERROR] {}", strerror(errno));
			break;
		}

		if(pfds[0].revents) {
			Conn *c;

			while((c = Listen(listen_fd))) {
				if(errno == EAGAIN) {
					close(c->getSocket());
					delete c;
				}
				socket_t cfd = c->getSocket();
				assert(c->getSocket() > 1 && "cfd greater than 1");

				if(connections.size() <= (size_t)cfd) {
					connections.resize(cfd + 1);
				}
				assert(connections.size() > cfd && connections.size() && "Connection vector resized to fit file descriptor");

				assert(!connections[cfd]);
				connections[cfd] = c;
			}

			assert(((c && c->getSocket() > 2 && c->getSocket() < connections.size()) || !c) && "Connection object created with valid file descriptor ");
		}

		assert(connections.size() > 0 && "Client added");
		for(size_t i = 1; i < pfds.size(); i++) {
			uint32_t conn_ready = pfds[i].revents;
			if(!conn_ready)
				continue;

			Conn *c = connections[pfds[i].fd];
			if (conn_ready & POLLIN) {
				assert((int)(c->state & ConnState::RECVING));

			//	handle_read(conn);  // application logic
				println("Can read {}\n", c->getSocket());
				c->Recv((byte *)buffer, sizeof(buffer));
				println("DATA RECEIVED: {}", buffer);
				memset(buffer, 0, sizeof(buffer));
            }
            if (conn_ready & POLLOUT) {
				assert((int)(c->state & ConnState::SENDING));

				println("Can write {}\n", c->getSocket());
				sview response = "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/plain\r\n"
  "Content-Length: 13\r\n"
  "Connection: close\r\n"
  "\r\n"                  // <-- end of headers
  "Hello, world!";
				c->Send(response);
				println("DATA SENT: {}", response);
//               assert(conn->want_write);
//                handle_write(conn); // application logic
            }
			
		}

		println("end of loop");
		/*
		Conn* A = nullptr;
		A = Listen(listen_fd);
		assert(A && "'New' connection works");

		byte conn_buff[sizeof(Conn)];
		Conn *B = nullptr;
		B = Listen(listen_fd, (Conn *)conn_buff);
		assert(B && B == (Conn *)conn_buff && "Connection at 'place' works");
		*/
	}

	return 0;
}
