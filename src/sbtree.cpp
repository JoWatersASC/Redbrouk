#include "sbtree.h"

namespace redbrouk
{

SBTNode *rotate_left(SBTNode *root);
SBTNode *rotate_right(SBTNode *root);
SBTNode *rotate_lr(SBTNode *root);
SBTNode *rotate_rl(SBTNode *root);
SBTNode *sbt_rotate(SBTNode *root);

//-----------------------------------------
// SBT Helpers
//-----------------------------------------

inline bool is_black(SBTNode *n) { return IS_NULL(n) || n->color; }

inline SBTNode* grandparent_of(SBTNode *n) { return n->parent->parent; }

inline SBTNode* sibling_of(SBTNode *n) {
	if(IS_NULL(n->parent))
		return &NILNODE;

	return n == n->parent->left ? n->parent->right : n->parent->left;
}

inline SBTNode* uncle_of(SBTNode *n) {
	return sibling_of(n->parent);
}

inline SBTNode* neice_of(SBTNode *n) {
	return n == n->parent->left ? sibling_of(n)->left : sibling_of(n)->right;
}

inline SBTNode* nephew_of(SBTNode *n) {
	return n == n->parent->right ? sibling_of(n)->left : sibling_of(n)->right;
}

/*
* Rotates node by bringing 'root' down to the left of its right child.
*/
SBTNode* rotate_left(SBTNode *root) {
	SBTNode *rch = root->right;
	root->right = rch->left;
	if(!IS_NULL(rch->left))
		rch->left->parent = root;

	if(!IS_NULL(rch))
		rch->parent = root->parent;
	if(root->parent) {
		if(root == root->parent->left)
			root->parent->left = rch;
		else
			root->parent->right = rch;
	}

	if(!IS_NULL(rch))
		rch->left = root;
	root->parent = rch;

	return rch;
}
/*
* Rotates node by bringing 'root' down to the right of its left child.
*/
SBTNode* rotate_right(SBTNode *root) {
	SBTNode *lch = root->left;
	root->left = lch->right;

	if(!IS_NULL(lch->right))
		lch->right->parent = root;

	if(!IS_NULL(lch))
		lch->parent = root->parent;
	if(root->parent) {
		if(root == root->parent->right)
			root->parent->right = lch;
		else
			root->parent->left = lch;
	}

	if(!IS_NULL(lch))
		lch->right = root;
	root->parent = lch;

	return lch;
}

SBTNode* rotate_lr(SBTNode *root) {
	rotate_left(root->left);
	return rotate_right(root);
}

SBTNode* rotate_rl(SBTNode *root) {
	rotate_right(root->right);
	return rotate_left(root);
}
/*
* Gets direction of node.
* If 'single' is true, get's the single level direction of node (parent)
* Otherwise, get's the direction relative to parent and grandparent
*/
inline DIRECTION get_dir(SBTNode *node, bool single = 0) {
	if( IS_NULL(node->parent) || (!single && IS_NULL(grandparent_of(node))) )
		return NONE;

	return (DIRECTION)(
		((!single && node->parent == grandparent_of(node)->right) &~ single) << 1 |
		(node == node->parent->right) |
		single << 2
	);
}
inline void transplant(SBTNode *a, SBTNode *b) {
	b->parent = a->parent;
	if(IS_NULL(a->parent))
		return;

	bool a_onleft = (get_dir(a, true) == L);
	if( a_onleft )
		a->parent->left  = b;
	else
		a->parent->right = b;
}

//-----------------------------------------------------
// SBTree functions (regular binary tree functions)
//-----------------------------------------------------
SBTNode** sbt_search(SBTNode **root, double _key) {
	if(!root)
		return nullptr;

	SBTNode *node;
	while(!IS_NULL(node = *root) && node->key != _key) {
		if(node->key < _key)
			root = &node->right;
		else
			root = &node->left;
	}

	return !IS_NULL(*root) ? root : nullptr;
}

SBTNode** sbt_insert(SBTNode **root, SBTNode *in_node) {
	if(!root) {
		*root = in_node;
		return root;
	}

	SBTNode *parent = *root, *node;
	double _key = in_node->key;

	while(!IS_NULL( (node = *root) ) && node->key != _key) {
		parent = *root;
		if(node->key < _key)
			root = &node->right;
		else
			root = &node->left;
	}
	if(node && node->key == _key)
		return nullptr;

	in_node->parent = parent;
	(*root) = in_node;
	return root;
}

/*
* Detaches a node 'root' from the tree and returns the deleted node.
*/
SBTNode* sbt_detach(SBTNode *root) {
	if(IS_NULL(root->right)) {
		transplant(root, root->left);
		return root;
	}
	SBTNode *successor = root->right;
	SBTNode *X = successor;

	while(!IS_NULL(successor->left)) {
		successor = successor->left;
	}

	X = successor->right;
	transplant(successor, successor->right);
	transplant(root, successor);
	successor->left = root->left;
	successor->right = root->right;

	if(!IS_NULL(successor->left))
		successor->left->parent = successor;
	if(!IS_NULL(successor->right))
		successor->right->parent = successor;

	return X;
}

void sbt_replace(SBTNode *oldn, SBTNode *newn) {
	if(IS_NULL(oldn) || IS_NULL(newn) || oldn->key != newn->key)
		throw;

	bool has_left   = !IS_NULL(oldn->left);
	bool has_right  = !IS_NULL(oldn->right);

	transplant(oldn, newn);

	newn->left = oldn->left;
	if( has_left )
		newn->left->parent = newn;

	newn->right = oldn->right;
	if( has_right )
		newn->right->parent = newn;

	newn->color = oldn->color;
}

SBTNode* sbt_at(SBTNode *root, ssize_t offset) {
	size_t idx = 0;
	return sbt_at(root, offset, idx);
}
SBTNode* sbt_at(SBTNode *root, ssize_t offset, size_t &index) {
	if(IS_NULL(root))
		return root;

	SBTNode *left = sbt_at(root->left, offset, index);
	if(!IS_NULL(left))
		return left;

	if(index == offset)
		return root;
	index++;

	SBTNode *right = sbt_at(root->right, offset, index);
	return right;
}

SBTNode* sbt_walk_forw(SBTNode* node) {
    if(IS_NULL(node))
        return nullptr;

    if(!IS_NULL(node->right)) {
        SBTNode* curr = node->right;
        while(!IS_NULL(curr->left))
			curr = curr->left;
        return curr;
    }

    SBTNode* p = node->parent;
    while(p && node == p->right) {
        node = p;
        p = p->parent;
    }

    return p;
}
SBTNode* sbt_walk_back(SBTNode* node) {
    if(IS_NULL(node))
        return nullptr;

    if(!IS_NULL(node->left)) {
        SBTNode* curr = node->left;
        while(!IS_NULL(curr->right))
			curr = curr->right;
        return curr;
    }

    SBTNode* p = node->parent;
    while(p && node == p->left) {
        node = p;
        p = p->parent;
    }

    return p;
}

SBTNode* sbt_walk(SBTNode *root, ssize_t offset) {
	SBTNode *out = root;

	RBTNode* (*walk_func) (RBTNode *) = &sbt_walk_forw;
	// std::function<RBTNode* (RBTNode *)> walk_func = sbt_walk_forw;
	if(offset < 0) {
		offset = ~offset;
		walk_func = &sbt_walk_back;
	}

	while(offset-- > 0)
		out = walk_func(out);

	return out;
}

SBTNode* sbt_rotate(SBTNode *root) {
	DIRECTION dir = get_dir(root);

	switch(dir) {
		case LR:
			rotate_left(root->parent); // left rotate on parent node
			root = root->left;
		case LL:
			rotate_right(grandparent_of(root)); //right rotate grandparent node
			break;

		case RL:
			rotate_right(root->parent); // right rotate on parent
			root = root->right;
		case RR:
			rotate_left(grandparent_of(root)); // left rotate on grandparent
			break;

		default:
			break;
	}

	return root->parent;
}

//-----------------------------------------------------
// RBTree functions (red-black tree functions)
//-----------------------------------------------------
void rbt_fix(RBTNode *node) {
	if(node->color == RBTNode::BLACK)
		return;

	while(!IS_NULL(node->parent) && node->parent->color == RBTNode::RED) {
		RBTNode *u = uncle_of(node);

		if(!IS_NULL(u) && u->color == RBTNode::RED) {
			//RECOLOR(node)
			node->parent->color = RBTNode::BLACK;
			grandparent_of(node)->color = RBTNode::RED;

			u->color = RBTNode::BLACK;
			node = grandparent_of(node);
		} else {
			RBTNode *new_child = sbt_rotate(node);
			new_child->color = RBTNode::BLACK;
			new_child->left->color = RBTNode::RED;
			new_child->right->color = RBTNode::RED;

			break;
		}
	}
}

RBTNode* rbt_insert(RBTNode **root, RBTNode *in_node) {
	RBTNode **inserted = sbt_insert(root, in_node);
	if(!inserted)
		return &NILNODE;

	RBTNode *node = *inserted;
	rbt_fix(node);

	if((*root)->parent)
		*root = (*root)->parent;
	(*root)->color = RBTNode::BLACK;

	return node;
}

// NEEDS TO BE CLEANED UP
void rbt_delete(RBTNode **root, RBTNode *del_node) {
	bool del_node_isroot = (*root == del_node);
	bool was_black = is_black(del_node);
	RBTNode *X, *Y = del_node;

	NILNODE.parent = Y;
	if(IS_NULL(del_node->left) && IS_NULL(del_node->right)) {
		Y = X = del_node->right;
		transplant(del_node, del_node->right);
	}
	else if(IS_NULL(del_node->right)) {
		Y = X = del_node->left;
		transplant(del_node, del_node->left);
		was_black = is_black(X);
		X->color = del_node->color;
	}
	else if(IS_NULL(del_node->left)) {
		Y = X = del_node->right;
		transplant(del_node, del_node->right);
		was_black = is_black(X);
		X->color = del_node->color;
	}
	else {
		Y = del_node->right;
		while(!IS_NULL(Y->left))
			Y = Y->left;

		X = Y->right;
		transplant(Y, X);
		transplant(del_node, Y);

		if(Y != del_node->right) {
			Y->right = del_node->right;
			Y->right->parent = Y;
		}
		Y->left = del_node->left;
		Y->left->parent = Y;

		was_black = is_black(Y);
		Y->color = del_node->color;
	}

	if( was_black )
		rbt_del_fix(*root, X);
	else if( del_node_isroot )
		*root = Y;
}

void rbt_del_fix(RBTNode *&root, RBTNode *node) {
	RBTNode *sib, *neice, *nephew;

	while(!IS_NULL(node->parent) && node->color == RBTNode::BLACK) {
		sib = sibling_of(node);
		neice = neice_of(node);
		nephew = nephew_of(node);

		enum { LEFT, RIGHT } SIDE = ( get_dir(node, true) == L ? LEFT : RIGHT );
		bool red_parent           = ( node->parent->color == RBTNode::RED );
		bool red_sibling          = ( sib->color == RBTNode::RED );
		bool double_black         = is_black(sib->left) && is_black(sib->right);
		bool red_neice            = !is_black(neice);
		bool red_nephew           = !is_black(nephew); // would be the same as saying 'else'

		if( red_parent ) {
			if( red_neice ) {
				if(SIDE == LEFT) {
					rotate_right(sib);
					rotate_left(node->parent);
				} else {
					rotate_left(sib);
					rotate_right(node->parent);
				}
				node->parent->color = RBTNode::BLACK;
			}
			else if(SIDE == LEFT) {
				rotate_left(node->parent);
			} else {
				rotate_right(node->parent);
			}

			break;
		}
		if( red_sibling ) {
			RBTNode *p = node->parent;
			if(SIDE == LEFT) {
				rotate_left(p);
			} else {
				rotate_right(p);
			}

			p->color = RBTNode::RED;
			sib->color = RBTNode::BLACK;
			break;
		}
		if( red_nephew ) {
			if(SIDE == LEFT)
				rotate_left(node->parent);
			else
				rotate_right(node->parent);
			nephew->color = RBTNode::BLACK;

			break;
		}
		if( red_neice ) {
			sbt_rotate(neice);
			neice->color = RBTNode::BLACK;
			break;
		}

		sib->color = RBTNode::RED;
		node = grandparent_of(node);
		if(IS_NULL(node))
			return;
	}
	node->color = RBTNode::BLACK;
	if(IS_NULL(node->parent))
		root = node;
}

} // namespace redbrouk
