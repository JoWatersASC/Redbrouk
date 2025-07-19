#include "kvt_hash_t.h"

namespace redbrouk {

HashSet::HashSet() {
	curr.buckets = std::make_unique<unique_ptr<HashSetNode>[]>(8);
	curr.mask = 7;
}
void HashSet::insert(HashSetNode *node) {
	const size_t pos = node->hash_val & curr.mask;

	node->next.reset(node);
	curr.buckets[pos].swap(node->next);
	curr.size++;
}
void HashSet::insert(std::string _data) {
	if(find(_data))
		return;

	HashSetNode *node = new HashSetNode(_data);
	insert(node);
}

HashSetNode* HashSet::find(string_view _data) {
	if( unique_ptr<HashSetNode> *match = table_find(curr, _data) )
		return match->get();

	if( unique_ptr<HashSetNode> *match = table_find(prev, _data) )
		return match->get();

	return nullptr;
}

unique_ptr<HashSetNode> HashSet::del(string_view _data) {
	auto table_del = [&] (Table &table, unique_ptr<HashSetNode> *target) {
		unique_ptr<HashSetNode> node = std::move(*target);
		target->swap(node->next);
		table.size--;

		return node;
	};

	if( unique_ptr<HashSetNode> *match = table_find(curr, _data) ) {
		return table_del(curr, match);
	}
	if( unique_ptr<HashSetNode> *match = table_find(prev, _data) ) {
		return table_del(prev, match);
	}

	return nullptr;
}

void HashSet::rehash() {
}
void HashSet::migrate() {
	prev = std::move(curr);
	curr.buckets = std::make_unique<unique_ptr<HashSetNode>[]>((curr.nbuckets + 1) * 2);
	migrate_pos = 0;
}
void HashSet::progress_rehash() {
}

unique_ptr<HashSetNode>* HashSet::table_find(Table &table, string_view _data) {
	if(!table.buckets)
		return nullptr;

	NodeDummy dummy(_data);

	const size_t pos = dummy.hash_val & table.mask;
	unique_ptr<HashSetNode> *bucket = &table.buckets[pos];
	
	HashSetNode *curr_node;
	while( (curr_node = bucket->get()) ) {
		if(curr_node->hash_val == dummy.hash_val && curr_node->key == dummy.data)
			return bucket;

		bucket = &curr_node->next;
	};

	return nullptr;
}

} // namespace redbrouk
