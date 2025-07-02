#ifndef REDBROUK_HASH_H
#define REDBROUK_HASH_H

#include <functional>

#include <cstdlib>
#include <cstddef>
#include <cassert>

#define XXH_INLINE_ALL
#include "lib/xxhash.h"

namespace redbrouk
{

using std::byte;
template <class... Args>
using bfunc = std::function<bool(Args...)>; // boolean function

// Restricted hash definition for simplicity and testing's sake
[[nodiscard]]
constexpr inline size_t genHash(const byte *data, size_t len) {
	return XXH64(data, len, 0);
}

typedef struct ih_node { // intrusive hash node
	ih_node *next;
	size_t hval = 0;
} iHNode;

typedef struct ih_table { // intrusive hash table
	iHNode **buckets = nullptr;
	union {
		size_t mask = 0;
		size_t nbuckets; // nbuckets is 1 less than number of buckets, to be in coherence w/ the mask
	};
	size_t size = 0;
} iHTab;

typedef struct ih_set {
	iHTab curr;
	iHTab prev;
	size_t migrate_pos = 0;
} iHSet;

extern size_t ihs_load; // intrusive hash set load factor
static inline iHTab *mk_ihtable(iHTab *place, size_t _size, iHNode **bplace) { //
	assert(_size !=  0 && ((_size - 1) & _size) == 0); // _size is a power of 2

	if(!place) {
		place = (iHTab *)malloc(sizeof(iHTab));
		// place = new iHTab
	}
	if(bplace) {
		memset(bplace, 0, _size);
	} else {
		bplace = (iHNode**)calloc(_size, sizeof(iHNode *));
		// bplace = new iHNode*[_size](0);
	}
	place->buckets = bplace;
	place->mask = _size - 1;
	place->size = 0;

	return place;
}
static inline iHSet *mk_ihset(iHSet *place = nullptr, iHNode **bplace = nullptr) {
	iHSet *out;
	if(place)
		out = new (place) iHSet{ {}, {}, 0 };
	else
		out = new iHSet{ {}, {}, 0 };

	mk_ihtable(&out->curr, 8, bplace);
	return out;
}

void iht_insert(iHTab *ht, iHNode *node);
iHNode **iht_get(const iHTab *ht, const iHNode *key, bfunc<const iHNode*, const iHNode*> eq);
iHNode *iht_del(iHTab *ht, iHNode **target);

// table migrate, migrate curr table to prev and reinitialize curr table as empty
void tmigrate(iHSet *hs);
iHNode *ihs_find(const iHSet *hs, const iHNode *key, bfunc<const iHNode*, const iHNode*> eq);
void ihs_insert(iHSet *hs, iHNode *node);
iHNode *ihs_del(iHSet *hs, iHNode *key, bfunc<const iHNode*, const iHNode*> eq);
void ihs_prehash(iHSet *hs);

} // namespace redbrouk

#endif
