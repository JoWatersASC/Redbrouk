#include "kvobj.h"
#include "kvt_map.h"
#include "kvt_string.h"
#include "kvt_tset.h"

#include <memory>

namespace redbrouk
{

IKVValtype IKVValtype::NIL;

KVObj::KVObj(KVTYPE _type) : m_type(_type) {
	switch(_type) {
		case KVTYPE::STRING:
			m_val = std::make_unique<String>();
			break;
		case KVTYPE::HASH:
			m_val = std::make_unique<iHMap>();
			break;
		case KVTYPE::TSET:
			m_val = std::make_unique<TSet>();
			break;
		default:
			break;
	}
}

} // namespace redbrouk
