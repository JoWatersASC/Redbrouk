#include "io.h"
#include "connection.h"
#include "network.h"

int main() {
	redbrouk::io_context iocon;
	iocon.init();
	iocon.main_loop();
}
