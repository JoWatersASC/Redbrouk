#ifndef REDBROUK_NETWORK_H
#define REDBROUK_NETWORK_H

#include <print>
#include <string_view>

#include <cstdint>
#include <cstddef>

#include <errno.h>
#include <poll.h>

#include <unistd.h>
#include <arpa/inet.h>

#include "connection.h"

namespace redbrouk
{

using std::byte;
using sview = std::string_view;

using socket_t = int;

namespace
{
	static socklen_t sock_len = sizeof(sockaddr_in);
	static socklen_t sock_len6 = sizeof(sockaddr_in6);

	int sock_on = 1;
}// anonymous namespace

struct endpoint {
	uint16_t port;
	sview addr;
};

inline struct sockaddr_in make_addr(const endpoint ep) {
	struct sockaddr_in addr = {};

	addr.sin_family = AF_INET;
	addr.sin_port = htons(ep.port);
	inet_pton(AF_INET, ep.addr.data(), &addr.sin_addr.s_addr);

	return addr;
}

inline struct sockaddr_in6 make_addr6(const endpoint ep) {
	struct sockaddr_in6 addr = {};

	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(ep.port);
	inet_pton(AF_INET6, ep.addr.data(), &addr.sin6_addr.s6_addr);

	return addr;
}

[[nodiscard]]
static inline socket_t make_listener(uint16_t _port, sview interface = "0.0.0.0", int bl = SOMAXCONN) {
	endpoint ep = { .port = _port, .addr = interface };

	sockaddr_in addr = make_addr(ep);
	
	int sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	assert(sock_fd != -1);
	// if(sock_fd == -1);
		//log error

	if(bind(sock_fd, (sockaddr *)&addr, sock_len) == -1) {
		std::println("[ERROR] Couldn't bind to socket: {}", strerror(errno));
		exit(1);
	}

	if(listen(sock_fd, bl) == -1) {
		std::println("[ERROR] Coudln't listen on socket: {}", strerror(errno));
		exit(1);
	}

	setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &sock_on, sizeof(sock_on));

	return sock_fd;
}

// Returns pointer to address of created Connection
[[nodiscard]]
static Conn *Accept(socket_t listen_fd, Conn* place = nullptr) {
	if(place)
		place->state = ConnState::ACCEPTING;

	sockaddr_in client_addr;
	socket_t conn_fd = accept4(listen_fd, (sockaddr *)&client_addr, &sock_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if(conn_fd == -1)
		return nullptr;
	setsockopt(conn_fd, SOL_SOCKET, SO_REUSEADDR, &sock_on, sizeof(sock_on));

	Conn *c;
	if(place && *((char *)place + sizeof(Conn) - 1))
		c = new (place) Conn(conn_fd);
	else
		c = new Conn(conn_fd);
	
	/* uint32_t ip = client_addr.sin_addr.s_addr;
		std::println("New client from {}.{}.{}.{}:{}", 
		ip & 255, (ip >> 8) & 255, (ip >> 16) & 255, ip >> 24,
        ntohs(client_addr.sin_port)
    ); */
	c->state = ConnState::OPEN | ConnState::RECVING;

	return c;
}

[[nodiscard]]
static inline Conn *Connect(uint16_t _port, sview _addr, Conn *place = nullptr) {
	socklen_t sock_len = sizeof(sockaddr_in);
	socket_t sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);

	endpoint ep{ .port = _port, .addr = _addr };
	sockaddr_in addr = make_addr(ep);

	if(connect(sock_fd, (sockaddr *)&addr, sock_len) == -1) { // connection unsuccessful
		/* LOG AND HANDLE ERROR */
	}
	setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &sock_on, sizeof(sock_on));
	Conn *c;

	c->state = ConnState::OPEN | ConnState::SENDING;

	return nullptr;
}

} // namespace redbrouk

#endif
