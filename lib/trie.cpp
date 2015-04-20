#include "trie.hpp"

std::vector<std::vector<size_t>> flatten(const trie & t) {
	std::vector<std::vector<size_t>> ret;
	t.for_each([&ret](auto && w) { ret.push_back(w); });
	return ret;
}

std::pair<size_t, size_t> total_size(const trie & t) {
	size_t count = 0;
	size_t total_count = 0;
	t.for_each([&count, &total_count](auto && w) {
		++count;
		total_count += w.size();
	});
	return {count, total_count};
}
