#ifndef REDBROUK_KVT_HASH_T_H
#define REDBROUK_KVT_HASH_T_H

#include "kvobj.h"
#include "src/utils.h"
#include <cstddef>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace redbrouk {

using std::unique_ptr;
using std::shared_ptr;
using std::string_view;


enum class HashType : uint8_t {
	SET,
	MAP
};

struct HashSetNode {
	HashSetNode(std::string _key) : key(std::move(_key)) {
		compute_hash();
	}

	HashSetNode *next = nullptr;
	size_t hash_val = 0;
	std::string key;

	// Modifiers
	void compute_hash() {
		hash_val = genHash((const byte *)key.c_str(), key.length());
	};
	void set_key(std::string _key) {
		key = std::move(_key);
		compute_hash();
	}
};

class HashSet : Valtype {
public:
	HashSet();
	HashSet(const HashSet &other) = delete;
	HashSet& operator=(const HashSet &other) = delete;
	~HashSet() { destroy(*this); }

	void emplace(std::string _data);
	void insert(HashSetNode *node);
	HashSetNode *del(string_view _data);
	HashSetNode* find(string_view _data);

	[[nodiscard]] const size_t size() const { return curr.size + prev.size; }
	void rehash();
	void detach() {
		curr.buckets.release();
		prev.buckets.release();

		curr.size = 0;
		prev.size = 0;

		migrate_pos = 0;
	}

	static void destroy(HashSet &set);

private:
	size_t migrate_pos = 0;

	using Bucket = HashSetNode*;
	struct Table {
		unique_ptr<Bucket[]> buckets;
		size_t size = 0;

		union {
			size_t mask = 0;
			size_t nbuckets;
		};

		void insert(HashSetNode *node) {
			const size_t pos = node->hash_val & mask;

			/*
			node->next = node;
			std::swap(buckets[pos], node->next);
			*/
			node->next = buckets[pos];
			buckets[pos] = node;
			size++;
		}
		HashSetNode *del(HashSetNode *&target) {
			if(!target)
				return nullptr;

			HashSetNode *node = target;
			target = target->next;
			size--;

			return node;
		}
		void take(Table& table, HashSetNode *&node) {
			insert(table.del(node));
		}

	} curr, prev;

	inline void migrate();
	void progress_rehash();
	HashSetNode* table_find(Table &table, string_view _data);
	static void destroy_table(Table &table);

	static size_t max_load;
	static size_t rehash_work;

	friend class HashMap;
	friend struct std::formatter<Table>;
	friend struct std::formatter<HashSet>;
};
using Set = HashSet;











struct HashMapNode {
	HashMapNode(std::string _key, std::string _val) : val(std::move(_val)), sn(std::move(_key)) {}

	HashSetNode sn;
	std::string val;

	// Accessors
	[[nodiscard]] size_t hash_val()  { return sn.hash_val; }
	[[nodiscard]] std::string &key() { return sn.key; }

	// Modifiers
	void compute_hash()            { sn.compute_hash(); };
	void set_key(std::string _key) { sn.set_key(_key); }
};

class HashMap : Valtype {
public:
	HashMap() { m_data.reserve(16); }
	void insert(std::string _key, std::string _val) {
		HashMapNode &node = m_data.emplace_back(_key, _val);
		m_set.insert(&node.sn);
	}
	HashMapNode *find(string_view _key) {
		HashSetNode *node = m_set.find(_key);
		if(!node)
			return nullptr;

		return mnfromsn(node);
	}
	HashMapNode *del(string_view _key) {
		HashSetNode *node = m_set.del(_key);
		if(!node)
			return nullptr;

		return mnfromsn(node);
	}

private:
	static HashMapNode *mnfromsn(HashSetNode *node) {
		return utils::container_of(node, &HashMapNode::sn);
	}
	static HashMapNode &mnfromsn_v(HashSetNode *node) {
		return *utils::container_of(node, &HashMapNode::sn);
	}

	std::vector<HashMapNode> m_data;
	HashSet m_set;

	static size_t max_load;
	static size_t rehash_work;

	friend struct std::formatter<HashSet::Table>;
	friend struct std::formatter<HashMap>;
};
using Dict = HashMap;

struct NodeDummy {
	NodeDummy(string_view _data) :
		data(_data),
		hash_val( genHash((const byte *)_data.data(), data.length()) ) {}

	HashSetNode *next;
	const string_view data;
	const size_t hash_val;
};

} // namespace redbrouk

static inline bool PRINTING_HS = false;
static inline bool PRINTING_HM = false;
MAKE_FORMATTER(redbrouk::HashSet::Table, {
	string buckets;

if(type.size == 0)
	buckets = "";
else
	buckets = [&] {
		std::string b_string;

		for(int i = 0; i <= type.nbuckets; i++) {
			if(PRINTING_HS || PRINTING_HM)
				b_string.append(std::format("\t  [{}] ", i));
			else
				b_string.append(std::format("\t[{}] ", i));
				redbrouk::HashSetNode *bucket = type.buckets[i];

			while( auto &elt = bucket ) {
				if(PRINTING_HM)
					b_string.append(std::format(
						"{} : {}, ",
						elt->key,
						redbrouk::HashMap::mnfromsn_v(elt).val
					));
				else
					b_string.append(std::format("{}, ", elt->key));
				bucket = bucket->next;
			}
			b_string.back() = '\n';
		}
		b_string.back() = '\n';

		return b_string;
	}();

if(PRINTING_HS || PRINTING_HM)
	str.assign(std::format(
		"\tSize: {}\n"
		"\tMask: {}\n"
		"\tNBuckets: {}\n"
		"\tBuckets:\n{}",
		type.size, type.mask, type.nbuckets + 1, buckets
	));

else
	str.assign(std::format(
		"Size: {}\n"
		"Mask: {}\n"
		"NBuckets: {}\n"
		"Buckets: \n{}",
		type.size, type.mask, type.nbuckets + 1, buckets
	));
})

MAKE_FORMATTER(redbrouk::HashSet, {
	const size_t size = type.curr.size + type.prev.size;
	const size_t mpos = type.migrate_pos;

	PRINTING_HS = true;
	str.assign(std::format(
		"Size: {}\n"
		"Migration Position: {}\n"
		"Current Table: \n{}\n"
		"Previous Table: \n{}",
		size, mpos, type.curr, type.prev
	));
	PRINTING_HS = false;
})
#endif
