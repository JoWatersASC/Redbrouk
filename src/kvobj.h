#ifndef REDBROUK_KVOBJ_H
#define REDBROUK_KVOBJ_H

#include "src/hash.h"
#include "src/utils.h"
#include "src/sbtree.h"

#include <memory>
#include <string>
#include <utility>

namespace redbrouk
{

class IKVValtype {
public:
	IKVValtype() = default;
	virtual ~IKVValtype() = default;
};
using Valtype = IKVValtype;

enum class KVTYPE : uint8_t {
	INIT = 0,
	STRING,
	HASH,
	TSET
};

class KVObj {
public:
	KVObj() : type(KVTYPE::INIT) {}
	KVObj(KVTYPE);

	iHNode hook;
	KVTYPE type;

	void rehash_hook() {
		hook.hval = genHash((const byte *)key.data(), key.length());
	}


	template <KVTYPE _type, typename... Args>
	auto make_val(Args&&... args) -> std::pair<KVTYPE, Valtype*>;
	/*
	template <typename... Args>
	auto make_val(Args&&... args) -> std::pair<KVTYPE, Valtype*> {
		return make_val<type>(std::forward<Args>(args)...);
	}
	*/

	std::string &get_key() { return key; }
	const std::string &get_key() const { return key; }
	Valtype &get_val()   { return *val.get(); }
	const Valtype &get_val() const { return *val.get(); }
	Valtype *get_val_p() { return val.get(); }
	const Valtype *get_val_p() const { return val.get(); }

	/* cpp 2x
	template <KVTYPE kvt>
	auto &get_val() {
		if constexpr (kvt string, hash, etc.) {
			return (String&, hash&, etc.&)*val.get();
		} else if...
	}
	can throw if kvt isn't same as type
	*/

private:
	std::string key;
	std::unique_ptr<Valtype> val;
};

class String;
class iHMap;
typedef struct tset TSet;

template <KVTYPE _type, typename... Args>
auto KVObj::make_val(Args&&... args) -> std::pair<KVTYPE, Valtype*> {
	std::tuple<KVTYPE, Valtype*> out = { type, val.release() };

	type = _type;
	if constexpr (_type == KVTYPE::STRING) {
		val = std::make_unique<String>(std::forward<Args>(args)...);
	}
	if constexpr (_type == KVTYPE::HASH) {
		val = std::make_unique<iHMap>(std::forward<Args>(args)...);
	}
	if constexpr (_type == KVTYPE::TSET) {
		val = std::make_unique<TSet>(std::forward<Args>(args)...);
	}

	return out;
}
/*
template <class KVT, typename... Args>
requires std::is_base_of_v<Valtype, KVT>
KVT *KVObj::make_val(Args&&... args) {
	KVT *out = val.release();
	val = std::make_unique<KVT>(std::forward<Args>(args)...);
	reurn out;
}

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
