#include "trie.hpp"

std::vector<std::vector<size_t>> flatten(const trie& t) {
	std::vector<std::vector<size_t>> ret;

	t.for_each([&ret](auto&& w) { ret.push_back(w); });

	return ret;
}
