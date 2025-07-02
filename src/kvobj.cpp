#include "kvobj.h"

namespace redbrouk
{

KVObj::KVObj(KVTYPE _type) : type(_type) {
	switch(_type) {
		case KVTYPE::STRING:
			val.emplace<std::string>();
			break;
		case KVTYPE::HASH:
			val.emplace<iHMap>();
			break;
		case KVTYPE::TSET:
			val.emplace<TSet>();
			break;
		default:
			break;
	}
}

} // namespace redbrouk
