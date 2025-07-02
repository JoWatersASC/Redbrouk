#include "kvt_map.h"
#include "src/hash.h"
#include "src/utils.h"

namespace redbrouk
{

namespace {
	std::string nil = "";
} // anonymous

[[nodiscard]]
bool IntrusiveHashMap::find(std::string& key) const {
	iHMPair entry;
	entry.node.hval = hash_func((const byte *)key.data(), key.length());
	entry.key.swap(key);

	iHNode *node = ihs_find(&m_table, &entry.node, [&](const iHNode *a, const iHNode *b) {
		return *a == *b;
	});

	return node;
}
std::string &IntrusiveHashMap::operator[](std::string &key) {
	iHMPair entry;
	entry.node.hval = hash_func((const byte *)key.data(), key.length());
	entry.key.swap(key);

	iHNode *node = ihs_find(&m_table, &entry.node, [&](const iHNode *a, const iHNode *b) {
		return *a == *b;
	});
	entry.key.swap(key);

	if(!node)
		return nil;

	return utils::container_of(node, &iHMPair::node)->val;
}

iHMPair *IntrusiveHashMap::emplace(std::string&& key, string&& val) {
	iHMPair *entry;
	if(!free_list.empty()) {
		entry = new (free_list.top()) iHMPair{ {}, key, val };
		free_list.pop();
	} else {
		entry = new (&m_elts[positions_used++]) iHMPair{ {}, key, val };
	}

	entry->node.hval = hash_func((byte *)entry->key.data(), entry->key.length());
	ihs_insert(&m_table, &entry->node);

	return entry;
}

iHMPair *IntrusiveHashMap::insert(iHMPair &&_pair) {
	return emplace(std::move(_pair.key), std::move(_pair.val));
}

iHMPair *IntrusiveHashMap::insert(iHMPair &_pair) {
	ihs_insert(&m_table, &_pair.node);
	return &_pair;
}

iHMPair *IntrusiveHashMap::remove(std::string &key) {
	// dummy node to find node for deletion
	iHMPair key_pair = {
		iHNode{ {}, hash_func((const byte *)key.data(), key.length()) },
		key, std::string{}
	};

	iHNode *found = ihs_del(&m_table, &key_pair.node, [&](const iHNode *a, const iHNode *b) {
		return *a == *b;
	});

	if(found) {
		iHMPair *p = utils::container_of(found, &iHMPair::node);
		free_list.push(p);
		return p;
		// find address of pair containing node and push to free list stack
	}

	return nullptr;
}

iHMPair *IntrusiveHashMap::remove(std::string &&key) {
	// dummy node to find node for deletion
	iHMPair key_pair = {
		iHNode{ {}, hash_func((const byte *)key.data(), key.length()) },
		key, std::string{}
	};

	iHNode *found = ihs_del(&m_table, &key_pair.node, [&](const iHNode *a, const iHNode *b) {
		return *a == *b;
	});

	if(found) {
		iHMPair *p = utils::container_of(found, &iHMPair::node);
		free_list.push(p);
		return p;
		// find address of pair containing node and push to free list stack
	}

	return nullptr;
}

} // namespace redbrouk
