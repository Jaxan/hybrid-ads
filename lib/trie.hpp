#pragma once

#include <boost/optional.hpp>

#include <stack>
#include <stdexcept>
#include <utility>
#include <vector>

///
/// \brief A Trie datastructure used to remove prefixes in a set of words
///
/// The datastructure only works for words over size_t. In principle the symbols
/// can be unbounded, however having very large symbols degrades the performance
/// a lot. Some random testing shows that for symbols <= 50 the performance is
/// similar to std::set (which is solving a different problem).
///
/// Tests : 1M words, avg words length 4 (geometric dist.), alphabet 50 symbols
/// trie reduction 58% in 1.15s
/// set  reduction 49% in 0.92s
///
/// I did not implement any iterators, as those are quite hard to get right.
/// There are, however, "internal iterators" exposed as a for_each() member
/// function (if only we had coroutines already...)
///
struct trie {
	/// \brief Inserts a word (given by iterators \p begin and \p end)
	/// \returns true if the element was inserted, false if already there
	template <typename Iterator> bool insert(Iterator&& begin, Iterator&& end) {
		if (begin == end) return false;

		size_t i = *begin++;
		if (i >= branches.size()) branches.resize(i + 1);

		auto& b = branches[i];
		if (b) return b->insert(begin, end);

		b = trie();
		b->insert(begin, end);
		count++;
		return true;
	}

	/// \brief Inserts a word given as range \p r
	/// \returns true if the element was inserted, false if already there
	template <typename Range> bool insert(Range const& r) {
		return insert(begin(r), end(r));
	}

	/// \p function is applied to all word (not to the prefixes)
	template <typename Fun> void for_each(Fun&& function) const {
		std::vector<size_t> word;
		return for_each_impl(std::forward<Fun>(function), word);
	}

	private:
	template <typename Fun>
	void for_each_impl(Fun&& function, std::vector<size_t>& word) const {
		if (count == 0) {
			const auto& cword = word;
			function(cword); // we don't want function to modify word
			return;
		}

		for (size_t i = 0; i < branches.size(); ++i) {
			auto const& b = branches[i];
			if (b) {
				word.push_back(i);
				b->for_each_impl(function, word);
				word.resize(word.size() - 1);
			}
		}
	}

	size_t count = 0;
	std::vector<boost::optional<trie>> branches;
};

/// \brief Flattens a trie \p t
/// \returns an array of words (without the prefixes)
std::vector<std::vector<size_t>> flatten(trie const& t);
