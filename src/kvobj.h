#ifndef REDBROUK_KVOBJ_H
#define REDBROUK_KVOBJ_H

#include "src/hash.h"
#include "src/utils.h"

#include <memory>
#include <string>
#include <utility>

namespace redbrouk
{

class IKVValtype {
public:
	IKVValtype() = default;
	virtual ~IKVValtype() = default;

	static IKVValtype NIL;
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
	KVObj() : m_type(KVTYPE::INIT) {}
	KVObj(KVTYPE);

	void rehash_hook() {
		m_hook.hval = genHash((const byte *)m_key.data(), m_key.length());
	}

	template <KVTYPE _type, typename... Args>
	auto make_val(Args&&... args) -> std::pair<KVTYPE, Valtype*>;
	/*
	template <m_typename... Args>
	auto make_val(Args&&... args) -> std::pair<KVTYPE, Valtype*> {
		return make_val<m_type>(std::forward<Args>(args)...);
	}
	*/

	// Accessors
	[[nodiscard]] iHNode* hook()                     { return &m_hook; }
	[[nodiscard]] const KVTYPE type()          const { return m_type; }
	[[nodiscard]] const std::string& get_key() const { return m_key; }
	[[nodiscard]] Valtype& val()                     { return (m_val.get() ? *m_val : Valtype::NIL); }
	[[nodiscard]] const Valtype& val()         const { return (m_val.get() ? *m_val : Valtype::NIL); }
	[[nodiscard]] Valtype* val_p()                   { return m_val.get(); }
	[[nodiscard]] const Valtype* val_p()       const { return m_val.get(); }

	// Mutators
	void set_key(std::string &new_key) noexcept {
		m_key = std::move(new_key);
		rehash_hook();
	}
	void set_key(std::string &&new_key) noexcept {
		m_key = new_key;
		rehash_hook();
	}

	/* cpp 17+
	template <KVTYPE kvt>
	auto &val() {
		if constexpr (kvt string, hash, etc.) {
			return (String&, hash&, etc.&)*m_val.get();
		} else if...
	}
	auto *val() {...} same thing
	can throw if kvt isn't same as m_type
	*/

private:
	iHNode m_hook;
	KVTYPE m_type;

	std::string m_key;
	std::unique_ptr<Valtype> m_val;

	friend class iHMap;
	friend KVObj* get_kvobj(iHNode*);
	friend KVObj& get_kvobj_v(iHNode*);
};
inline KVObj* get_kvobj(iHNode *hook)   { return utils::container_of(hook, &KVObj::m_hook); }
inline KVObj& get_kvobj_v(iHNode *hook) { return *utils::container_of(hook, &KVObj::m_hook); }

class String;
class iHMap;
typedef struct tset TSet;

template <KVTYPE _type, typename... Args>
auto KVObj::make_val(Args&&... args) -> std::pair<KVTYPE, Valtype*> {
	std::tuple<KVTYPE, Valtype*> out{ m_type, m_val.release() };

	m_type = _type;
	if constexpr (_type == KVTYPE::STRING) {
		m_val = std::make_unique<String>(std::forward<Args>(args)...);
	}
	if constexpr (_type == KVTYPE::HASH) {
		m_val = std::make_unique<iHMap>(std::forward<Args>(args)...);
	}
	if constexpr (_type == KVTYPE::TSET) {
		m_val = std::make_unique<TSet>(std::forward<Args>(args)...);
	}

	return out;
}

/*
template <class KVT, m_typename... Args>
requires std::is_base_of_v<Valtype, KVT>
KVT *KVObj::make_val(Args&&... args) {
	KVT *out = m_val.release();
	val = std::make_unique<KVT>(std::forward<Args>(args)...);
	reurn out;
}
*/

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
