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
	iHNode *found;
	found = ihs_find(&tst->mts_mp, (const iHNode*)&dummy, eq);
	if(!found) {
		return nullptr;
	}

	TSTNode *out = utils::container_of(found, &TSTNode::mpnode);

	return out;
}

bool ts_insert(TSet *tst, TSTNode *node) {
	RBTNode *inserted, **found = sbt_search(&tst->stm_root, node->tnode.key);

	if(found) {
		auto container  = utils::container_of(*found, &TSTNode::tnode);
		container->next = node;
		inserted = *found;
	} else {
		inserted = rbt_insert(&tst->stm_root, &node->tnode);
	}

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
bool ts_insertn(TSet *tst, std::string &&_name, double _score) {
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

	if(del_node->next) {
		if(&del_node->tnode == tst->stm_root)
			tst->stm_root = &del_node->next->tnode;

		sbt_replace(&del_node->tnode, &del_node->next->tnode);
		del_node->next = nullptr;
	} else {
		rbt_delete(&tst->stm_root, &del_node->tnode);
	}

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

TSTNode *ts_walk(TSTNode *n, ssize_t offset, TSTNode *head) {
	RBTNode *sbt_walk_forw(RBTNode *);
	RBTNode *sbt_walk_back(RBTNode *);

	RBTNode *found = &n->tnode;
	RBTNode* (*walk_func) (RBTNode *) = &sbt_walk_forw;
	// std::function<RBTNode* (RBTNode *)> walk_func = sbt_walk_forw;
	if(offset < 0) {
		offset = ~offset;
		walk_func = &sbt_walk_back;
	}

	while(offset-- > 0) {
		if(n->next) {
			n = n->next;
		} else {
			if(head)
				found = &head->tnode;

			found = walk_func(found);
			n = utils::container_of(found, &TSTNode::tnode);
			if(head)
				head = n;
		}
	}

	return found ? n : nullptr;
}

TSTNode *ts_at(TSet *tst, ssize_t offset) {
	TSTNode *node  = utils::container_of(tst->stm_root, &TSTNode::tnode);
	RBTNode *tnode = &node->tnode;
	size_t size = ts_size(tst);

	assert(offset < size && "Offset less than size of tset");
	assert((offset >= 0 || size + offset >= 0) && "Negative offset abs val less than size of tset");

	while(!IS_NULL(tnode->left))
		tnode = tnode->left;

	node = utils::container_of(tnode, &TSTNode::tnode);
	node = ts_walk(node, offset);

	return node;
}

} // namespace redbrouk
