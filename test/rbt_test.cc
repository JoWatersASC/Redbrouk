#include <cassert>
#include <print>

#include "sbtree.h"
#include "kvt_tset.h"

using namespace redbrouk;

size_t left_len(SBTNode *node);

int main(int argc, char *argv[]) {
	std::println("{}", sizeof(struct sbt_node));

	double root_key = 150.f;
	RBTNode *root = new sbt_node{ nullptr, {0}, root_key, sbt_node::BLACK };
	printf("root == %p\n", root);

	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 779, sbt_node::RED });
	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 180, sbt_node::RED });
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");
	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 448, sbt_node::RED });
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");
	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 460, sbt_node::RED });
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");
	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 368, sbt_node::RED });
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");
	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 259, sbt_node::RED });
	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 816, sbt_node::RED });
	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 648, sbt_node::RED });
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");

	rbt_delete(&root, root->right);
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");

	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 84, sbt_node::RED });
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");
	
	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 77, sbt_node::RED });
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");
	
	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 100, sbt_node::RED });
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");

	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 200, sbt_node::RED });
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");
	
	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 309, sbt_node::RED });
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");
	
	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 250, sbt_node::RED });
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");

	rbt_insert(&root, new RBTNode{ root, { &NILNODE, &NILNODE }, 1000, RBTNode::RED });
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");

	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 500, RBTNode::RED });
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");

	rbt_insert(&root, new sbt_node{ root, { &NILNODE, &NILNODE }, 1001, RBTNode::RED });
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");

	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");

	rbt_delete(&root, root->left);
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");

	rbt_delete(&root, root->right->left);
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");

	rbt_delete(&root, root->right);
	print_sbt(root);
	std::println("\n--------------------------------------------------------------------------------");
	/*{
		printf("%p\n", sbt_insert(&root, new sbt_node{ root, {0}, 1000, sbt_node::RED }));
		printf("%p\n", sbt_insert(&root, new sbt_node{ root, {0}, 500, sbt_node::RED }));
		printf("%p\n", sbt_insert(&root, new sbt_node{ root, {0}, 1001, sbt_node::RED }));


		print_sbt(root);

		rotate_left(root->right->right);
		std::println("\n");
		print_sbt(root);
	}*/

	/*{
		printf("%p\n", sbt_insert(&root, new sbt_node{ root, {0}, -1000, sbt_node::RED }));
		printf("%p\n", sbt_insert(&root, new sbt_node{ root, {0}, -500, sbt_node::RED }));
		printf("%p\n", sbt_insert(&root, new sbt_node{ root, {0}, -1001, sbt_node::RED }));

		rotate_right(root->left->left);
		std::println("\n");
		print_sbt(root);
	}*/

	/*{
		SBTNode *t1 = root->left, *t2 = root->left->right, *t3 = root->right->right;

		root->left = sbt_detach(t1);
		std::println("\n");
		// print_sbt(root);

		root->left->right = sbt_detach(t2);
		std::println("\n");
		// print_sbt(root);

		root->right->right= sbt_detach(t3);
		std::println("\n");
		// print_sbt(root);
	}*/
}

size_t left_len(SBTNode *node) {
	size_t out = 1;
	while(node->left && ++out)
		node = node->left;

	return out;
}
