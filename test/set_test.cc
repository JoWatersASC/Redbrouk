#include "kvt_hash_t.h"
#include <print>

using namespace std;
using redbrouk::Set;

int main(int argc, char *argv[]) {
	Set s;
	s.insert("Hello");
	s.insert("oppenheimer");
	s.insert("World");
	s.insert("Joshua");
	s.insert("christopher");
	s.insert("zayden");
	s.insert("William");
	s.insert("Jadyn");
	s.insert("Frances");
	s.insert("Lauren");
	s.insert("Waters");
	s.insert("wendigo");
	s.insert("goku");
	s.insert("addison");
	s.insert("James");

	std::string str = format("{}", s);
	std::println("{}", s);
}
