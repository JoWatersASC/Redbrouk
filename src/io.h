#ifndef REDBROUK_IO_H
#define REDBROUK_IO_H

#include <vector>
#include <string>
#include <cstdlib>
#include <cstdint>

#include <poll.h>

namespace redbrouk
{

using socket_t = int;
using std::byte;
using sview = std::string_view;

class Conn;

constexpr size_t MAX_PFDS = 1024;
enum ev_type {
	NIL = 0,
	CAN_RD,
	CAN_WR
};

struct fdEvent {
	ev_type state = NIL;
	Conn *conn = nullptr;
};

typedef struct io_context {
	std::vector<struct pollfd> pfds;

	void init(uint16_t _port = 16000);
	void main_loop();

	void stop() { running = false; }

	bool running = true;
	socket_t highFd = -1;

	byte recv_buf[1024 * 4];
	byte send_buf[1024 * 4];
} ioc; // struct io_context

struct Response {
    uint32_t status = 0;
    std::vector<uint8_t> data;
};

enum {
    RES_OK = 0,
    RES_ERR = 1,
    RES_NX = 2,
};

static void handle_read(Conn *conn);
static void handle_write(Conn *conn);
static void do_request(std::vector<std::string> &cmd, Response &out);
static int  try_request(Conn *conn);
static int32_t parse_req(const std::byte*, size_t, std::vector<std::string_view>&);

void sigint_handler(int sig_num);
} // namespace redbrouk

#endif
