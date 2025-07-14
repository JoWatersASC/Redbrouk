#ifndef REDBROUK_CONNECTION_H
#define REDBROUK_CONNECTION_H

#include <span>

#include <cassert>
#include <cstring>

#include <arpa/inet.h>
#include <unistd.h>

namespace redbrouk
{

using socket_t = int;
using std::byte;

enum class ConnState : uint8_t {
	NONE		= 0,
	CONNECTING	= (1 << 0),
	ACCEPTING	= (1 << 1),
	OPEN		= (1 << 2),
	CLOSED		= (1 << 3),
	SENDING		= (1 << 4),
	RECVING		= (1 << 5),
	ERR 		= (1 << 6)
};

using std::span;
struct endpoint;

class Conn {
public:
	Conn(socket_t _m_fd = -1) : m_fd(_m_fd) {}
	~Conn();

	int recv(byte *ibuff, size_t len);
	int brecv(byte *ibuff, size_t len); // Blocking reception

	int send(byte *obuff, size_t len);
	int bsend(const span<byte> obuff); // Blocking send

	[[nodiscard]]
	const socket_t get_socket() const { return m_fd; }

	void setPeerAddr(const sockaddr_in &peer_addr);

	void Close() { ::close(m_fd); }

	ConnState state = ConnState::NONE;

	byte *in_buff;
	byte *ot_buff;

	off_t in_start = 0, in_end = 0;
	off_t ot_start = 0, ot_end = 0;

	byte* in_data() { return in_buff + in_start; }
	byte* ot_data() { return ot_buff + ot_start; }
	const off_t in_size() const { return in_end - in_start; }
	const off_t ot_size() const { return ot_end - ot_start; }

private:
	endpoint* peer_ep;
	socket_t m_fd; // socket file descriptor
};

// Operators for ConnState enum
[[nodiscard]]
constexpr inline ConnState operator|(ConnState a, ConnState b) noexcept {
	return (ConnState)((uint8_t)a | (uint8_t)b);
}
[[nodiscard]]
constexpr inline ConnState operator&(ConnState a, ConnState b) noexcept {
	return (ConnState)((uint8_t)a & (uint8_t)b);
}
[[nodiscard]]
constexpr inline ConnState operator~(ConnState c) noexcept {
	return (ConnState)(~(uint8_t)c);
}

constexpr inline ConnState& operator|=(ConnState& a, ConnState b) noexcept {
	a = a | b;
	return a;
}
constexpr inline ConnState operator&=(ConnState& a, ConnState b) noexcept {
	a = a & b;
	return b;
}

} // namespace redbrouk

#endif
