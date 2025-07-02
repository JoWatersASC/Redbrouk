#include <cassert>
#include <print>
#include <queue>
#include <iostream>
#include <iomanip>
#include <random>

#include "hash.h"
#include "map.h"
#include "tset.h"
#include "sbtree.h"
#include "utils.h"

using namespace redbrouk;

size_t left_len(SBTNode *node);

int main(int argc, char *argv[]) {
	iHMap m;
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<> dist(1, 1100);
	std::uniform_real_distribution<> rdist(1, 1000);
	std::uniform_int_distribution<> cdist(0, 25);

	TSet t;
	std::string tstn_name;
	double score;
	int count = 0;
	auto remake_args = [&](std::string &&_name, bool flag = true) {
		if(!flag) {
			_name.clear();
			int l = dist(gen);

			for(int i = 0; i < l; i++) {
				_name.push_back('a' + cdist(gen));
			}
		}
	};

	remake_args("joshua");
	score = 779.144;
	ts_insertn(&t, tstn_name, score);
	std::println("\033[97m\n[{}]Score: {}\n", count++, score);
	print_sbt(t.stm_root);

	remake_args("andrew");
	score = 349.353;
	ts_insertn(&t, tstn_name, score);
	std::println("\033[97m\n[{}]Score: {}\n", count++, score);
	print_sbt(t.stm_root);

	remake_args("waters");
	score = 180.907;
	ts_insertn(&t, tstn_name, score);
	std::println("\033[97m\n[{}]Score: {}\n", count++, score);
	print_sbt(t.stm_root);

	remake_args("alex");
	score = 448.773;
	ts_insertn(&t, tstn_name, score);
	std::println("\033[97m\n[{}]Score: {}\n", count++, score);
	print_sbt(t.stm_root);

	remake_args("maxwell");
	score = 560.755;
	ts_insertn(&t, tstn_name, score);
	std::println("\033[97m\n[{}]Score: {}\n", count++, score);
	print_sbt(t.stm_root);

	remake_args("lonzo");
	score = 368.535;
	ts_insertn(&t, tstn_name, score);
	std::println("\033[97m\n[{}]Score: {}\n", count++, score);
	print_sbt(t.stm_root);

	remake_args("jadyn");
	score = 259.27;
	ts_insertn(&t, tstn_name, score);
	std::println("\033[97m\n[{}]Score: {}\n", count++, score);
	print_sbt(t.stm_root);

	remake_args("bailey");
	score = 59.27;
	ts_insertn(&t, tstn_name, score);
	std::println("\033[97m\n[{}]Score: {}\n", count++, score);
	print_sbt(t.stm_root);

	remake_args("nando");
	score = 1002.27;
	ts_insertn(&t, tstn_name, score);
	std::println("\033[97m\n[{}]Score: {}\n", count++, score);
	print_sbt(t.stm_root);

	remake_args("conne");
	score = 247.27;
	ts_insertn(&t, tstn_name, score);
	std::println("\033[97m\n[{}]Score: {}\n", count++, score);
	print_sbt(t.stm_root);

	for(int i = 0; i < 20; i++) {
		std::string nam = "";
		for(int j = 0; j < 8; j++) {
			nam += 'a' + cdist(gen);
		}

		score = rdist(gen);
		remake_args(std::move(nam));
		score = rdist(gen);
		ts_insertn(&t, tstn_name, score);
		std::println("\033[97m\n[{}]Score: {}\n", count++, score);
		print_sbt(t.stm_root);
	}

	TSTNode *node = ts_at(&t, 4);
	std::println("[4] {} {}", node->name, node->tnode.key);
	node = ts_at(&t, 8);
	std::println("[8] {} {}", node->name, node->tnode.key);
	node = ts_at(&t, 0);
	std::println("[0] {} {}", node->name, node->tnode.key);
	//node = ts_at(&t, 41);
	//std::println("[21] {} {}", node->name, node->tnode.key);

	node->tnode = *sbt_walk(t.stm_root, 0);
	std::println("[Walk 0] {} {}", node->name, node->tnode.key);
	node->tnode = *sbt_walk(t.stm_root, 3);
	std::println("[Walk 3] {} {}", node->name, node->tnode.key);
	node->tnode = *sbt_walk(t.stm_root, -1);
	std::println("[Walk -1] {} {}", node->name, node->tnode.key);
	node->tnode = *sbt_walk(t.stm_root, -8);
	std::println("[Walk -8] {} {}", node->name, node->tnode.key);
	return 0;

	remake_args("joshua");
	ts_deleten(&t, tstn_name);
	print_sbt(t.stm_root);

	std::println("\n___________________________________________________\n");

	remake_args("bailey");
	ts_deleten(&t, tstn_name);
		print_sbt(t.stm_root);
	std::println("\n___________________________________________________\n");

	remake_args("jadyn");
	ts_deleten(&t, tstn_name);
		print_sbt(t.stm_root);
	std::println("\n___________________________________________________\n");

	remake_args("lonzo");
	ts_deleten(&t, tstn_name);
		print_sbt(t.stm_root);
	std::println("\n___________________________________________________\n");

	remake_args("maxwell");
	ts_deleten(&t, tstn_name);
		print_sbt(t.stm_root);
	std::println("\n___________________________________________________\n");

	remake_args("alex");
	ts_deleten(&t, tstn_name);
		print_sbt(t.stm_root);
	std::println("\n___________________________________________________\n");

	remake_args("waters");
	ts_deleten(&t, tstn_name);
		print_sbt(t.stm_root);
	std::println("\n___________________________________________________\n");
	remake_args("andrew");
	ts_deleten(&t, tstn_name);
		print_sbt(t.stm_root);
	std::println("\n___________________________________________________\n");

	free(t.mts_mp.curr.buckets);

}

size_t left_len(SBTNode *node) {
	size_t out = 1;
	while(node->left && ++out)
		node = node->left;

	return out;
}
