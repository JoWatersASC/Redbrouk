#include "hash.h"

namespace redbrouk
{

void iht_insert(iHTab *ht, iHNode *node) {
	size_t pos = node->hval & ht->mask;

	node->next = ht->buckets[pos];
	ht->buckets[pos] = node;
	ht->size++;
}

iHNode **iht_get(const iHTab *ht, const iHNode *key_node, bfunc<const iHNode*, const iHNode*> eq) {
	if(!ht->buckets)
		return nullptr;

	size_t pos = key_node->hval & ht->mask;
	iHNode **bucket = &ht->buckets[pos];
	
	iHNode *curr;
	while((curr = *bucket)) {
		if(curr->hval == key_node->hval && eq(curr, key_node))
			return bucket;

		bucket = &curr->next;
	};

	return nullptr;
}

iHNode *iht_del(iHTab *ht, iHNode **target) {
	iHNode *node = *target;
	*target = node->next;
	ht->size--;

	return node;
}

namespace { // anonymous namespace
	constexpr size_t prehash_work = 64;
	// auto default_prhstop = [] (size_t& work_done) -> bool { return work_done >= prehash_work; };
	// op dependent rehash (e.g. old_size / 1000)
	// time dependent rehash
	// load factor adaptive (e.g. progress < expected progress: hash step ++)
	// 
	// batch rehash of all elts in bucket (may change rehash work size to map more to buckets)
	// rehash by entries per bucket (> load buckets go first)
	// forced full rehash
}
size_t ihs_load = 1;
void tmigrate(iHSet *hs) {
	hs->prev = hs->curr;
	auto p = hs->prev;
	mk_ihtable(&hs->curr, (hs->curr.nbuckets + 1) * 2, NULL);
	hs->migrate_pos = 0;
}

iHNode *ihs_find(const iHSet *hs, const iHNode *key, bfunc<const iHNode*, const iHNode*> eq) {
	iHNode **match = iht_get(&hs->curr, key, eq);
	if(!match)
		match = iht_get(&hs->prev, key, eq);

	return match ? *match : nullptr;
}
iHNode *ihs_del(iHSet *hs, iHNode *key, bfunc<const iHNode*, const iHNode*> eq) {
	iHNode **match;

	if( (match = iht_get(&hs->curr, key, eq)) ) {
		return iht_del(&hs->curr, match);
	}
	if( (match = iht_get(&hs->prev, key, eq)) ) {
		return iht_del(&hs->prev, match);
	}

	return nullptr;
}

void ihs_insert(iHSet *hs, iHNode *node) {
	if(!hs->curr.buckets)
		mk_ihtable(&hs->curr, 8, NULL);

	iht_insert(&hs->curr, node);
	if(!hs->prev.buckets) { // only migrate if the previous bucket set is empty
		size_t new_nbuckets = (hs->curr.nbuckets + 1) * ihs_load;
		if(hs->curr.size >= new_nbuckets) {
			tmigrate(hs);
		}
	}
}

void ihs_prehash(iHSet *hs) { // specifies when rehashing ends
	size_t work_done = 0;

	while(work_done < prehash_work && hs->prev.size > 0) {
		iHNode **bucket_head = &hs->prev.buckets[hs->migrate_pos];
		if(!bucket_head) {
			hs->migrate_pos++;
			continue;
		}

		iht_insert(&hs->curr, iht_del(&hs->prev, bucket_head));
		work_done++;
	}

	if(hs->prev.size == 0 && hs->prev.buckets) { // no more elts in prev, all moved to curr, free prev's memory
		free(hs->prev.buckets);
		hs->prev = {};
	}
}

} // namespace redbrouk
