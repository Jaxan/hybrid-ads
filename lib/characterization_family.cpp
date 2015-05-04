#include "characterization_family.hpp"
#include "trie.hpp"

#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

#include <functional>
#include <stack>
#include <utility>

using namespace std;

characterization_family create_seperating_family(const adaptive_distinguishing_sequence & sequence,
                                                 const separating_matrix & sep_matrix) {
	const auto N = sequence.CI.size();
	// all words (global/local) for all states
	vector<trie> suffixes(N);

	// all global separating sequences, which we will add to a state in the end ...
	trie all_global_separating_words;

	// ... to these particualr states
	vector<bool> state_needs_global_suffixes(N, false);


	// First we accumulate the kind-of-UIOs and the separating words we need. We will do this with a
	// breath first search.
	stack<pair<word, reference_wrapper<const adaptive_distinguishing_sequence>>> work;
	work.push({{}, sequence});
	while (!work.empty()) {
		auto word = work.top().first;
		const adaptive_distinguishing_sequence & node = work.top().second;
		work.pop();

		// On a leaf, we need to add the accumulated word as suffix (this is more or less a UIO).
		// And, if needed, we also need to augment the set of suffixes (for all pairs).
		if (node.children.empty()) {
			for (auto && p : node.CI) {
				const auto state = p.second;
				suffixes[state].insert(word);
			}

			for (auto && p : node.CI) {
				for (auto && q : node.CI) {
					const auto s = p.second;
					const auto t = q.second;
					if (s == t) continue;

					const auto & sep_word = sep_matrix[s][t];

					suffixes[s].insert(sep_word);
					all_global_separating_words.insert(sep_word);
					state_needs_global_suffixes[s] = true;
				}
			}

			continue;
		}

		// add some work
		for (auto && i : node.word) word.push_back(i);        // extend the word
		for (auto && c : node.children) work.push({word, c}); // and visit the children with word
	}


	// Then we flatten them into a characterization family.
	characterization_family ret(N);

	for (state s = 0; s < N; ++s) {
		auto & current_suffixes = suffixes[s];
		ret[s].local_suffixes = flatten(current_suffixes);

		if (state_needs_global_suffixes[s]) {
			all_global_separating_words.for_each(
			    [&current_suffixes](auto w) { current_suffixes.insert(w); });
		}

		ret[s].global_suffixes = flatten(current_suffixes);
	}

	return ret;
}
