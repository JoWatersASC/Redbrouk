#include "kvt_hash_t.h"

namespace redbrouk {

size_t HashSet::max_load = 1;
size_t HashSet::rehash_work = 128;

HashSet::HashSet() {
	curr.buckets = std::make_unique<unique_ptr<HashSetNode>[]>(8);
	curr.mask = 7;
}
void HashSet::insert(HashSetNode *node) {
	curr.insert(node);

	if(prev.size == 0) {
		const size_t new_nbuckets = (curr.nbuckets + 1) * max_load;
		if(curr.size >= new_nbuckets) {
			migrate();
		}
	}

	progress_rehash();
}
void HashSet::emplace(std::string _data) {
	if(find(_data))
		return;

	HashSetNode *node = new HashSetNode(_data);
	insert(node);
}

HashSetNode* HashSet::find(string_view _data) {
	progress_rehash();
	if( unique_ptr<HashSetNode> *match = table_find(curr, _data) )
		return match->get();

	if( unique_ptr<HashSetNode> *match = table_find(prev, _data) )
		return match->get();

	return nullptr;
}

unique_ptr<HashSetNode> HashSet::del(string_view _data) {
	progress_rehash();
	if( unique_ptr<HashSetNode> &match = *table_find(curr, _data) ) {
		return curr.del(match);
	}
	if( unique_ptr<HashSetNode> &match = *table_find(prev, _data) ) {
		return prev.del(match);
	}

	return nullptr;
}

void HashSet::rehash() {
	const size_t curr_nbuckets = (curr.nbuckets + 1);
	const size_t new_cap = ( size() > (curr_nbuckets * max_load) ) ? curr_nbuckets * 2 : curr_nbuckets;

	Table new_table = {
		.buckets = std::make_unique< unique_ptr<HashSetNode>[] >(new_cap),
		.size = 0,
		.mask = new_cap - 1
	};

	size_t pos = 0;
	while(prev.size > 0) {
		unique_ptr<HashSetNode> &node = prev.buckets[pos];
		if(!node) {
			pos++;
			continue;
		}

		new_table.take(prev, node);
	}

	pos = 0;
	while(curr.size > 0) {
		unique_ptr<HashSetNode> &node = curr.buckets[pos];
		if(!node) {
			pos++;
			continue;
		}

		new_table.take(curr, node);
	}

	curr = std::move(new_table);
}
inline void HashSet::migrate() {
	prev = std::move(curr);

	const size_t new_nbuckets = (curr.nbuckets + 1) * 2;
	curr.buckets = std::make_unique<unique_ptr<HashSetNode>[]>(new_nbuckets);
	curr.mask = new_nbuckets - 1;
	curr.size = 0;

	migrate_pos = 0;
}
void HashSet::progress_rehash() {
	size_t work_done = 0;

	while(work_done < rehash_work && prev.size > 0) {
		unique_ptr<HashSetNode> &node = prev.buckets[migrate_pos];
		if(!node) {
			migrate_pos++;
			continue;
		}

		curr.take(prev, node);
		work_done++;
	}
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
