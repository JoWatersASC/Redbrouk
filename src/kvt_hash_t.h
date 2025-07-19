#ifndef REDBROUK_KVT_HASH_T_H
#define REDBROUK_KVT_HASH_T_H

#include "kvobj.h"
#include "src/utils.h"
#include <concepts>
#include <cstddef>
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
	HashMapNode *mnfromsn(HashSetNode *node) {
		return utils::container_of(node, &HashMapNode::sn);
	}
	HashMapNode &mnfromsn_v(HashSetNode *node) {
		return *utils::container_of(node, &HashMapNode::sn);
	}

	std::vector<HashMapNode> m_data;
	HashSet m_set;
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

#endif
