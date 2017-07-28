#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

///
/// \brief A Trie datastructure used to remove prefixes in a set of words.
/// Insert-only. Iteration over the structure only uses longest matches.
///
/// Tests : 1M words, avg words length 4 (geometric dist.), alphabet 50 symbols
/// trie reduction 58% in 0.4s
/// set  reduction 49% in 1.1s
///
/// I did not implement any iterators, as those are quite hard to get right.
/// There are, however, "internal iterators" exposed as a for_each() member
/// function (if only we had coroutines already...)
///
/// TODO: implement `bool member(...)`
///
template <typename T> struct trie {
	/// \brief Inserts a word (given by iterators \p begin and \p end)
	/// \returns true if the element was inserted, false if already there
	template <typename Iterator> bool insert(Iterator && begin, Iterator && end) {
		if (!node) {
			node.reset(new trie_node());

			if (begin == end) {
				return true;
			}
		}

		return node->insert(begin, end);
	}

	/// \brief Inserts a word given as range \p r
	/// \returns true if the element was inserted, false if already there
	template <typename Range> bool insert(Range const & r) { return insert(begin(r), end(r)); }

	/// \brief Applies \p function to all word (not to the prefixes)
	template <typename Fun> void for_each(Fun && function) const {
		if (node) {
			node->for_each(std::forward<Fun>(function));
		} else {
			// empty set, so we don't call the function
		}
	}

	/// \brief Empties the complete set
	void clear() { node.reset(nullptr); }

  private:
	struct trie_node;
	std::unique_ptr<trie_node> node = nullptr;

	// A node always contains the empty word
	struct trie_node {
		template <typename Iterator> bool insert(Iterator && begin, Iterator && end) {
			if (begin == end) return false;

			T i = *begin++;
			auto it = find(i);

			if (it != data.end() && it->first == i) {
				return it->second.insert(begin, end);
			}

			// else, does not yet exist
			it = data.emplace(it, i, trie_node());
			it->second.insert(begin, end);
			return true;
		}

		template <typename Fun> void for_each(Fun && function) const {
			std::vector<T> word;
			return for_each_impl(std::forward<Fun>(function), word);
		}

	  private:
		template <typename Fun> void for_each_impl(Fun && function, std::vector<T> & word) const {
			if (data.empty()) {
				// we don't want function to modify word
				const auto & cword = word;
				function(cword);
			}

			for (auto const & kv : data) {
				// for each letter, we extend the word, recurse and remove extension.
				word.push_back(kv.first);
				kv.second.for_each_impl(function, word);
				word.resize(word.size() - 1);
			}
		}

		typename std::vector<std::pair<T, trie_node>>::iterator find(T const & key) {
			return std::lower_bound(
			    data.begin(), data.end(), key,
			    [](std::pair<T, trie_node> const & kv, T const & k) { return kv.first < k; });
		}

		std::vector<std::pair<T, trie_node>> data;
	};
};

/// \brief Flattens a trie \p t
/// \returns an array of words (without the prefixes)
template <typename T> std::vector<std::vector<T>> flatten(trie<T> const & t) {
	std::vector<std::vector<T>> ret;
	t.for_each([&ret](std::vector<T> const & w) { ret.push_back(w); });
	return ret;
}

/// \brief Returns size and total sum of symbols
template <typename T> std::pair<size_t, size_t> total_size(trie<T> const & t) {
	size_t count = 0;
	size_t total_count = 0;
	t.for_each([&count, &total_count](std::vector<T> const & w) {
		++count;
		total_count += w.size();
	});
	return {count, total_count};
}
