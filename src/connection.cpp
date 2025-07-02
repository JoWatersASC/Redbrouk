#include <cstdlib>
#include <errno.h>

#include "connection.h"
#include "network.h"

namespace redbrouk
{

int Conn::Recv(byte *ibuff, size_t len) {
	ssize_t n = read(m_fd, ibuff, len);

	if(n < 0) {
		if(errno == EAGAIN)
			return 0;

		state = ConnState::CLOSED | ConnState::ERR;
		return -1;
	}

	if(n == len) {
		state = ConnState::RECVING;
	}

	in_end += n;
	return n;
}


int Conn::Send(byte *obuff, size_t len) {
	ssize_t n = write(m_fd, obuff, len);

	if(n < 0) {
		if(errno == EAGAIN)
			return 0;

		state = ConnState::CLOSED | ConnState::ERR;
		return -1;
	}

	if(n == len) {
		state = ConnState::RECVING;
	}

	ot_start += n;
	return n;
}

int Conn::BRecv(byte *ibuff, size_t len) {
	size_t bytes_read = 0;
	
	while(bytes_read < len) {
		ssize_t n = recv(m_fd, ibuff + bytes_read, len - bytes_read, 0);

		if(n < 0) {
			state = ConnState::CLOSED | ConnState::ERR;
			close(m_fd);
			return n;
		} else if (n == 0) {
			break;
		}

		bytes_read += n;
	}

	state = ConnState::SENDING;
#ifdef DEBUG
	std::println("Received {} bytes", bytes_read);
#endif
	return bytes_read;
}

int Conn::BSend(span<byte> obuff) {
	size_t len = obuff.size();
	assert(len > 0);
	size_t bytes_sent = 0;

	while(bytes_sent < len) {
		ssize_t n = send(m_fd, obuff.data() + bytes_sent, len - bytes_sent, 0);

		if(n < 0) {
			// if errno == Eintr continue
			return -1;
		} else if (n == 0) {
			break;
		}

		bytes_sent += n;
	}

	state = ConnState::RECVING;

	return bytes_sent;
}

void Conn::setPeerAddr(const sockaddr_in& peer_addr) {
	uint16_t pport = ntohs(peer_addr.sin_port);
	sview paddr;

	auto buff = (char *)malloc(32);
	inet_ntop(AF_INET, &peer_addr, buff, sizeof(peer_addr));

	peer_ep = new endpoint{ .port = pport, .addr = buff };
}

Conn::~Conn() { delete peer_ep; }

} // namespace redbrouk
