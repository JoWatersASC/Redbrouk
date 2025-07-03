#ifndef REDBROUK_KVT_STRING_H
#define REDBROUK_KVT_STRING_H

#include "src/kvobj.h"
#include <string>

namespace redbrouk {

class String : public Valtype, public std::string {
public:
	using std::string::string;
	String(const std::string &s) : std::string(s) {}
	String(std::string &&s) : std::string(std::move(s)) {}
};

} // namespace redbrouk

#endif
