#ifndef REDBROUK_TSET_H
#define REDBROUK_TSET_H

#include <string>

#include "kvobj.h"
#include "src/sbtree.h"

namespace redbrouk
{

/* TSET NODE - Node for tset type, has intrusive nodes for balanced tree and hash kvt_map.
 * Can be indexed by score through tnode and by name through mpnode.
 * Needs constructor due to std::string usage, will change in future
*/
typedef struct tst_node {
	iHNode mpnode;
	RBTNode tnode;
	std::string name;
} TSTNode;

typedef struct tset : Valtype {
	using IKVValtype::IKVValtype;
	RBTNode *stm_root = nullptr;
	iHSet mts_mp;
} TSet;

inline size_t ts_size(TSet *tst) {
	return tst->mts_mp.curr.size + tst->mts_mp.prev.size;
}

// *n means do <function> by name instead of pointer to node
bool ts_insert(TSet *tst, TSTNode *node);
bool ts_insertn(TSet *tst, std::string &_name, double _score);
bool ts_delete(TSet *tst, TSTNode *node, bool);
bool ts_deleten(TSet *tst, std::string_view _name);
bool ts_update(TSet *tst, TSTNode *node, double _score);

TSTNode *ts_find(TSet *tst, std::string_view _name); // Find a node in a tset by name
TSTNode *ts_seek(TSet *tst, double _score); // Find a node in a tset by score or closest score > '_score'
TSTNode *ts_at(TSet *tst, ssize_t offset); // Find a node by offset in order

static TSTNode *mk_tstn(std::string &_name, double _score, TSTNode *place = nullptr) {
	TSTNode *out;
	if(place)
		out = new (place) TSTNode;
	else
		out = new TSTNode;

	out->tnode = { nullptr , { &NILNODE, &NILNODE }, _score, RBTNode::RED };
	out->mpnode.next = nullptr;
	out->mpnode.hval = genHash((const byte *)_name.data(), _name.length());
	out->name = std::move(_name);

	return out;
}
inline void del_tstn(TSTNode *n) {
	delete n;
}

static tset *mk_tset(tset *place = nullptr, ih_node **bplace = nullptr) {
	tset *out;

	if(place)
		out = new (place) tset;
	else
		out = new tset;
	mk_ihset(&out->mts_mp, bplace);

	return out;
}
// tset function types:
//  single pair by name
//  single pair by score
//  range of pairs by score
//  range of pairs by name
/*
* TSet_entry {
* 	integral_t score;
* 	string mem
* };
*
* TSet {
*	tree_set score-mem;
*	hashmap  mem-score;
* }
*/

} // namespace redbrouk

#endif
