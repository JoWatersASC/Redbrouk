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

	unique_ptr<HashSetNode> next = nullptr;
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

class HashSet : Valtype {
public:
	HashSet();
	void insert(std::string _data);
	void insert(HashSetNode *node);
	HashSetNode* find(string_view _data);
	unique_ptr<HashSetNode> del(string_view _data);
	void rehash();

private:
	size_t migrate_pos = 0;
	struct Table {
		unique_ptr<
			unique_ptr<HashSetNode>[]
		>buckets;

		union {
			size_t mask = 0;
			size_t nbuckets;
		};

		size_t size = 0;
	} curr, prev;

	void migrate();
	void progress_rehash();

	unique_ptr<HashSetNode>* table_find(Table &table, string_view _data);

	friend class HashMap;
	friend struct std::formatter<Table>;
	friend struct std::formatter<HashSet>;
};
using Set = HashSet;

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
		unique_ptr<HashSetNode> node = m_set.del(_key);
		if(!node)
			return nullptr;

		return mnfromsn(node.release());
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
	buckets = "EMPTY";
else
	buckets = [&] {
		std::string b_string;

		for(int i = 0; i <= type.nbuckets; i++) {
			if(PRINTING_HS)
				b_string.append(std::format("\t\[{}] ", i));
			else
				b_string.append(std::format("\t[{}] ", i));
			unique_ptr<redbrouk::HashSetNode> *bucket = &type.buckets[i];

			while( auto &elt = *bucket ) {
				if(PRINTING_HM)
					b_string.append(std::format(
						"{} : {}, ",
						elt->key,
						redbrouk::HashMap::mnfromsn_v(elt.get()).val
					));
				else
					b_string.append(std::format("{}, ", elt->key));
				bucket = &(*bucket)->next;
			}

			b_string.pop_back();
			b_string.back() = '\n';
		}

		return b_string;
	}();

if(PRINTING_HS)
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
})
#endif
