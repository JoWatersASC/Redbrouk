#ifndef REDBROUK_MAP_H
#define REDBROUK_MAP_H

#include <string>
#include <stack>

#include "kvobj.h"

using std::string;

namespace redbrouk
{

class alignas(64) iHMap : public Valtype {
public:
	using IKVValtype::IKVValtype;
	iHSet table;
};

typedef struct ihm_pair { // string to string hash map entry pairs
	iHNode node;
	string key;
	string val;
} iHMPair;

[[nodiscard]]
constexpr bool operator==(const iHNode &a, const iHNode &b) {
	iHMPair *first  = utils::container_of((iHNode *)&a, &iHMPair::node);
	iHMPair *second = utils::container_of((iHNode *)&b, &iHMPair::node);

	return first->key == second->key;
}

constexpr ssize_t MAX_IMP_ELTS = 1024 * 16; // maximum entries for intrusive map
class IntrusiveHashMap { // class wrapper around intrusive hash maps
public:
	IntrusiveHashMap() {
		clear();
	}

	// hash function
	std::function<size_t (const byte*, size_t)> hash_func = genHash;

	// Capacity
	[[nodiscard]] size_t get_size() const { return m_table.curr.size + m_table.prev.size; }
	[[nodiscard]] bool is_empty()   const { return !get_size(); }
	[[nodiscard]] bool is_full()    const { return get_size() == MAX_IMP_ELTS; }

	// Element Access
	[[nodiscard]]
	bool find(std::string& key) const;
	[[nodiscard]]
	std::string &operator[](std::string &key);

	// Modification
	iHMPair *emplace(std::string&&, string&&);
	iHMPair *insert(iHMPair &&_pair);
	iHMPair *insert(iHMPair &_pair);
	iHMPair *remove(std::string &key);
	iHMPair *remove(std::string &&key);

	void clear() {
		m_table = {};
		free_list = {};
		positions_used = 0;
	} // need to make memory safe due to underlying data using c memory management

private:
	iHSet m_table;
	iHMPair m_elts[MAX_IMP_ELTS]; // can also have dynamically allocated buffer
	std::stack<iHMPair *> free_list; // stack of all indices that are free
	// fill the stack from empty, pull from stack if available, if not, take from positions and increment,
	// push to free_list when deleting a node
	size_t positions_used = 0; 
};

} // namespace redbrouk

#endif // ifndef REDBROUK_MAP_H
