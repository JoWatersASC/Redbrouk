#include "kvt_hash_t.h"
#include <print>

using namespace std;
using redbrouk::Set;

int main(int argc, char *argv[]) {
	Set s;
	s.emplace("Hello");
	s.emplace("oppenheimer");
	s.emplace("World");
	s.emplace("Joshua");
	s.emplace("christopher");
	s.emplace("zayden");
	s.emplace("William");
	s.emplace("Jadyn");
	s.emplace("Frances");
	s.emplace("Lauren");
	s.emplace("Waters");
	s.emplace("wendigo");
	s.emplace("goku");
	s.emplace("addison");
	s.emplace("James");
	s.emplace("Alexander");
	s.emplace("Matthew");
	s.emplace("Hotep");
	s.emplace("Umar");

	std::string str = format("{}", s);
	std::println("{}", s);
}
