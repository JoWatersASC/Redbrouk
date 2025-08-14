#ifndef REDBROUK_SBTREE_H
#define REDBROUK_SBTREE_H
#include <cassert>
#include <print>

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <compare>
#include <iostream>
#include <sys/types.h>

namespace redbrouk {

/* Direction of node
 *
 * L  = left child of parent.
 * R  = right child of parent.
 * LL = left child of parent that is left child.
 * LR = right child of parent that is left child.
 * RL = left child of paretn that is right child.
 * RR = right child of parent that is right child.
 */
enum DIRECTION : uint8_t { LL = 0, LR, RL, RR, L, R, NONE };

/* BALANCED TREE NODE - implemented solely as red black for now, implement avl later.
 * Plan to use pointer tagging to combine parent and color enum.
 *  May explicitly alignas(32) for better simd and cache friendliness.
 * Can also do tight array intrusive rb tree May move key to
 * container as well
 */
typedef struct sbt_node {
	sbt_node *parent;

	union {
		struct {
			sbt_node *left;
			sbt_node *right;
		};

		sbt_node *children[2];
	};

	double key;
	enum { RED, BLACK } color : 1 = RED;
} SBTNode, RBTNode; // , AVLNode;

[[nodiscard]]
inline std::partial_ordering operator<=>(const SBTNode &a, const SBTNode &b) {
	return a.key <=> b.key;
}

inline SBTNode NILNODE{ nullptr, { nullptr, nullptr }, 0, sbt_node::BLACK };
inline bool IS_NULL(SBTNode *N) { return !N || N == &NILNODE; }

SBTNode** sbt_search(SBTNode **root, double _key);
SBTNode** sbt_insert(SBTNode **root, SBTNode *in_node);
SBTNode*  sbt_detach(SBTNode  *root);
SBTNode*  sbt_at(SBTNode *root, ssize_t offset);
SBTNode*  sbt_at(SBTNode *root, ssize_t offset, size_t &index); 
SBTNode*  sbt_walk(SBTNode *root, ssize_t offset);
void      sbt_replace(SBTNode *oldn, SBTNode *newn);

inline SBTNode* sbt_min(SBTNode *root) {
	while(root->left)
		root = root->left;

	return root;
}
inline SBTNode* sbt_max(SBTNode *root) {
	while(root->right)
		root = root->right;

	return root;
}
// can split search size in half by seeing how many nodes per subtree

RBTNode *rbt_insert(RBTNode **root, RBTNode *in_node);
void rbt_delete(RBTNode **root, RBTNode *del_node);
void rbt_fix(RBTNode *node);
void rbt_del_fix(RBTNode *&root, RBTNode *X);

#define RED_COLOR "\033[31m"
#define BLACK_COLOR "\033[0m"

static void print_sbt_help(SBTNode *node, int &count, int indent = 0) {
	if (IS_NULL(node))
		return;

	// Print right subtree first (top of output)
	print_sbt_help(node->right, count, indent + 8);

	if (indent)
		std::cout << std::setw(indent) << ' ';

	std::cout << (!node->color ? "\033[31m" : "\033[97m") << '[' << count++ << "]. " << node->key << "\n";

	// Print left subtree (bottom of output)
	print_sbt_help(node->left, count, indent + 4);
}
static void print_sbt(SBTNode *node, int indent = 0) {
	int count = 0;
	print_sbt_help(node, count, indent);
}

} // namespace redbrouk

#endif
