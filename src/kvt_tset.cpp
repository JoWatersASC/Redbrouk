#include "sbtree.h"
#include "kvt_tset.h"
#include "sbtree.h"
#include "src/hash.h"
#include "src/utils.h"

namespace redbrouk
{


// Equalty function for TSTNodes' intrusive hash nodes
bool tstn_hneq(const iHNode *a, const iHNode *b) {
	return utils::container_of((iHNode *)a, &TSTNode::mpnode)->name ==
			utils::container_of((iHNode *)b, &TSTNode::mpnode)->name;
}

TSTNode *ts_find(TSet *tst, std::string_view _name) {
	iHNode dummy{ nullptr, genHash((const byte *)_name.data(), _name.length()) };

	auto eq = [&](const iHNode *a, const iHNode *b) -> bool {
		return utils::container_of((iHNode *)a, &TSTNode::mpnode)->name == _name;
	};
	iHNode *inserted;
	inserted = ihs_find(&tst->mts_mp, (const iHNode*)&dummy, eq);
	if(!inserted) {
		return nullptr;
	}

	TSTNode *out = utils::container_of(inserted, &TSTNode::mpnode);

	return out;
}

bool ts_insert(TSet *tst, TSTNode *node) {
	RBTNode *inserted = rbt_insert(&tst->stm_root, &node->tnode);
	if(IS_NULL(inserted))
		return false;

	ihs_insert(&tst->mts_mp, &node->mpnode);
	return true;
}
bool ts_insertn(TSet *tst, std::string &_name, double _score) {
	TSTNode *in_node = mk_tstn(_name, _score);

	if(ts_insert(tst, in_node))
		return true;

	delete in_node;
	return false;
}
bool ts_delete(TSet *tst, TSTNode *del_node, bool reclaim_mem = false) {
	if(!del_node)
		return false;

	ihs_del(&tst->mts_mp, &del_node->mpnode, tstn_hneq);
	rbt_delete(&tst->stm_root, &del_node->tnode);

	if(reclaim_mem)
		del_tstn(del_node);
	return true;
}
bool ts_deleten(TSet *tst, std::string_view _name) {
	TSTNode *del_node = ts_find(tst, _name);
	return ts_delete(tst, del_node);
}

bool ts_update(TSet *tst, TSTNode *node, double _score) {
	if(!ts_delete(tst, node))
		return false;

	node->tnode.key   = _score;
	node->tnode.left  = &NILNODE;
	node->tnode.right = &NILNODE;
	node->tnode.color = RBTNode::RED;

	ts_insert(tst, node);

	return true;
}

TSTNode *ts_seek(TSet *tst, double _score) {
	RBTNode *node = tst->stm_root;
	RBTNode *out = nullptr;
	if(!node)
		return nullptr;

	while(node) {
		if(_score < node->key) {
			node = node->left;
		} else if(_score > node->key) {
			out = node;
			node = node->right;
		} else {
			return utils::container_of(node, &TSTNode::tnode);
		}
	}

	return nullptr;
}

TSTNode *ts_walk(TSTNode *n, ssize_t offset) {
	if(SBTNode *found = sbt_walk(&n->tnode, offset))
		return utils::container_of(found, &TSTNode::tnode);
	return nullptr;
}
TSTNode *ts_at(TSet *tst, ssize_t offset) {
	size_t size = ts_size(tst);
	assert(offset < size && "Offset less than size of tset");
	assert((offset >= 0 || size + offset >= 0) && "Negative offset abs val less than size of tset");

	int64_t index = 0;
	if(offset < 0)
		offset = size + offset;

	RBTNode *found = sbt_at(tst->stm_root, offset);
	return utils::container_of(found, &TSTNode::tnode);
}

} // namespace redbrouk
