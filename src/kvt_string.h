#ifndef REDBROUK_KVT_STRING_H
#define REDBROUK_KVT_STRING_H

#include "src/kvobj.h"
#include <string>

namespace redbrouk {

class String : Valtype, public std::string {};

} // namespace redbrouk

#endif
