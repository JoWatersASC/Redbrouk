#include <print>
#include "sbtree.h"
#include "src/utils.h"

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

inline bool
is_black(SBTNode *n) { return IS_NULL(n) || n->color; }

inline SBTNode *
grandparent_of(SBTNode *n) { return n->parent->parent; }

inline SBTNode *
sibling_of(SBTNode *n) {
	if(IS_NULL(n->parent))
		return &NILNODE;

	return n == n->parent->left ? n->parent->right : n->parent->left;
}

inline SBTNode *
uncle_of(SBTNode *n) {
	return sibling_of(n->parent);
}

inline SBTNode *
neice_of(SBTNode *n) {
	return n == n->parent->left ? sibling_of(n)->left : sibling_of(n)->right;
}

inline SBTNode *
nephew_of(SBTNode *n) {
	return n == n->parent->right ? sibling_of(n)->left : sibling_of(n)->right;
}

/*
* Rotates node by bringing 'root' down to the left of its right child.
*/
SBTNode *rotate_left(SBTNode *root) {
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
SBTNode *rotate_right(SBTNode *root) {
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

SBTNode *rotate_lr(SBTNode *root) {
	rotate_left(root->left);
	return rotate_right(root);
}

SBTNode *rotate_rl(SBTNode *root) {
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
SBTNode **sbt_search(SBTNode **root, double _key) {
	if(!root)
		return nullptr;

	SBTNode *node;
	while((node = *root) && node->key != _key) {
		if(node->key < _key)
			root = &node->right;
		else
			root = &node->left;
	}

	return root;
}

SBTNode **sbt_insert(SBTNode **root, SBTNode *in_node) {
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
* Detaches a node 'root' from the tree and returns the node it's been replaced by.
*/
SBTNode *sbt_detach(SBTNode *root) {
	if(IS_NULL(root->right)) {
		transplant(root, root->left);
		return root->left;
	}
	SBTNode *new_root = root->right;

	while(!IS_NULL(new_root->left)) {
		new_root = new_root->left;
	}
	transplant(new_root, new_root->right);
	if(get_dir(new_root, true) == L)
		new_root->parent->left = new_root->right;

	transplant(root, new_root);
	new_root->left = root->left;

	return new_root;
}
SBTNode *sbt_at(SBTNode *root, ssize_t offset) {
	size_t idx = 0;
	return sbt_at(root, offset, idx);
}
SBTNode *sbt_at(SBTNode *root, ssize_t offset, size_t &index) {
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

static SBTNode *sbt_walk_forw(SBTNode* node) {
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
static SBTNode *sbt_walk_back(SBTNode* node) {
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

SBTNode *sbt_walk(SBTNode *root, ssize_t offset) {
	SBTNode *out = root;

	if(offset < 0) {
		offset = ~offset;

		while(offset-- > 0)
			out = sbt_walk_back(out);
	} else {
		while(offset-- > 0)
			out = sbt_walk_forw(out);
	}

	return out;
}

SBTNode *sbt_rotate(SBTNode *root) {
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

RBTNode *rbt_insert(RBTNode **root, RBTNode *in_node) {
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
	auto fix_root = [&] {
		while(!IS_NULL((*root)->parent))
			*root = (*root)->parent;
	};
	bool del_node_isroot = (*root == del_node);
	bool was_black;

	//2 children
	if(!IS_NULL(del_node->left) && !IS_NULL(del_node->right)) {
		std::println("Branch 1");
		RBTNode *successor = del_node->right;

		while(!IS_NULL(successor->left))
			successor = successor->left;

		// has right child
		// have to move right child into its place
		RBTNode *scsr_repln = successor->right; // successor replacement node
		RBTNode *scsr_p = successor->parent;

		was_black = (successor->color == RBTNode::BLACK);
		bool first_ch = (successor->parent == del_node);

		if( first_ch ) {// case that successor is just the first right child
			scsr_repln->parent = successor;
		} else {
			transplant(successor, successor->right);

		}

		successor->color = del_node->color;
		transplant(del_node, successor);
		successor->left = del_node->left;
		if( !first_ch )
			successor->right = del_node->right;

		if(!IS_NULL(successor->left))
			successor->left->parent = successor;
		if(!IS_NULL(successor->right))
			successor->right->parent = successor;

		if( first_ch ) {
			successor->right = scsr_repln;
			scsr_repln->parent = successor;
		} else {
			scsr_p->left = scsr_repln;
			scsr_repln->parent = scsr_p;
			scsr_repln->color = successor->color;
		}
		if( del_node_isroot )
			*root = successor;

		NILNODE.color = RBTNode::BLACK;
		if( was_black )
			rbt_del_fix(scsr_repln);
		return;
	}
	//1 child
	if(!IS_NULL(del_node->left)) {
		std::println("Branch 2");

		was_black = (del_node->left->color == RBTNode::BLACK);
		bool on_left = (get_dir(del_node, true) == L);

		del_node->left->parent = del_node->parent;
		if(!IS_NULL(del_node->parent)) {
			if( on_left )
				del_node->parent->left = del_node->left;
			else
				del_node->parent->right = del_node->left;
		} else { // if root
			(*root) = del_node->left;
			(*root)->color = RBTNode::BLACK;
			return;
		}

		if( was_black )
			rbt_del_fix(del_node->left);
		else
			del_node->left->color = RBTNode::BLACK;

		if( del_node_isroot )
			*root = del_node->left;
		return;
	}
	if(!IS_NULL(del_node->right)) {
		std::println("Branch 3");

		was_black = (del_node->right->color == RBTNode::BLACK);
		bool on_left = (get_dir(del_node, true) == L);

		del_node->right->parent = del_node->parent;
		if(!IS_NULL(del_node->parent)) {
			if( on_left )
				del_node->parent->left = del_node->right;
			else
				del_node->parent->right = del_node->right;
		} else { // if root
			(*root) = del_node->right;
			(*root)->color = RBTNode::BLACK;
			return;
		}

		if( was_black )
			rbt_del_fix(del_node->right);
		else
			del_node->right->color = RBTNode::BLACK;

		if( del_node_isroot )
			*root = del_node->right;
		return;
	}

	//cases where del_node has no children
	if( del_node_isroot ) { // del_node is the root
		std::println("Branch 4");
		*root = nullptr;
		return;
	}

	std::println("Branch 5");
	if(get_dir(del_node, true) == L)
		del_node->parent->left = &NILNODE;
	else
		del_node->parent->right = &NILNODE;
	if(!is_black(del_node))
		return;

	std::println("Branch 5.5");
	NILNODE.color  = RBTNode::BLACK;
	NILNODE.parent = del_node->parent;
	rbt_del_fix(&NILNODE);
	
	fix_root();
}

void rbt_del_fix(RBTNode *node) {
	RBTNode *sib, *neice, *nephew;
	RBTNode dummy{ 0 }; // dummy node to break out of while loop on case 4

	while(!IS_NULL(node->parent) && node->color == RBTNode::BLACK) {
		sib = sibling_of(node);
		neice = neice_of(node);
		nephew = nephew_of(node);

		bool red_sibling        = sib->color == RBTNode::RED;
		bool two_black_children = is_black(sib->left) && is_black(sib->right);
		bool red_neice          = !is_black(neice);
		bool red_nephew         = !is_black(nephew); // would be the same as saying 'else'

		if( red_sibling ) {
			if(get_dir(node, true) == L) {
				rotate_left(node->parent);
				sib = node->parent->right;
			}
			else {
				rotate_right(node->parent);
				sib = node->parent->right;
			}

			if(!IS_NULL(node->parent)) {
				node->parent->color = RBTNode::RED;

				if(!IS_NULL(grandparent_of(node)))
					grandparent_of(node)->color = RBTNode::BLACK;
			}
			if(is_black(sib->left) && is_black(sib->right)) {
				sib->color = RBTNode::RED;
				node = node->parent;
			}
		}
		else if( two_black_children ) { // black neice and black nephew
			sib->color = RBTNode::RED;
			node = node->parent;
		}
		else if( red_neice && !red_nephew ){
			if(get_dir(node, true) == L)
				rotate_right(sib);
			else
				rotate_left(sib);

			sib->color = RBTNode::RED;
			sib->parent->color = RBTNode::BLACK;
		} else if( red_nephew ) {
			nephew->color = RBTNode::BLACK;

			if(get_dir(node, true) == L) {
				rotate_left(node->parent);
			} else
				rotate_right(node->parent);

			sib->color = node->parent->color;
			node->parent->color = RBTNode::BLACK;
			node = &dummy;
		} // should maybe switch red_neice and red_nephew cases as if red_nephew fails, all that's needed to know is red_neice
	
	}
	node->color = RBTNode::BLACK;
// Always change node's color to black
}

} // namespace redbrouk
