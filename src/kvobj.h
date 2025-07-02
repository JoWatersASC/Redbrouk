#ifndef REDBROUK_KVOBJ_H
#define REDBROUK_KVOBJ_H

#include "src/hash.h"
#include "src/kvt_map.h"
#include "src/kvt_tset.h"
#include "src/utils.h"

#include <memory>
#include <string>
#include <utility>

namespace redbrouk
{

enum class KVTYPE : uint8_t {
	INIT = 0,
	STRING,
	HASH,
	TSET
};
class IKVValtype { public: virtual ~IKVValtype() = default; };
using Valtype = IKVValtype;

class KVObj {
public:
	KVObj() : type(KVTYPE::INIT) {}
	KVObj(KVTYPE);

	iHNode hook;
	KVTYPE type;

private:
	std::string key;
	std::unique_ptr<Valtype> val;
};

inline KVObj *get_kvobj(iHNode *hook) {
	return utils::container_of(hook, &KVObj::hook);
}

struct LookupDummy {
	/*
	LookupDummy(std::string_view _key) : key(_key) {
		hook.next = nullptr;
		hook.hval = genHash((const byte *)key.data(), key.length());
	}
	*/
	iHNode hook;
	std::string_view key;
};

} // namespace redbrouk

#endif // ifndef REDBROUK_KVOBJ_H
