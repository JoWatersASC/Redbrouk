#include "kvobj.h"
#include "kvt_map.h"
#include "kvt_string.h"
#include "kvt_tset.h"
#include <memory>

namespace redbrouk
{

KVObj::KVObj(KVTYPE _type) : type(_type) {
	switch(_type) {
		case KVTYPE::STRING:
			val = std::make_unique<String>();
			break;
		case KVTYPE::HASH:
			val = std::make_unique<iHMap>();
			break;
		case KVTYPE::TSET:
			val = std::make_unique<TSet>();
			break;
		default:
			break;
	}
}

} // namespace redbrouk
