#include "characterization_family.hpp"
#include "trie.hpp"

#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

#include <functional>
#include <stack>
#include <utility>

using namespace std;

characterization_family create_seperating_family(const adaptive_distinguishing_sequence & sequence,
                                                 const splitting_tree & separating_sequences) {
	const auto N = sequence.CI.size();

	vector<trie> suffixes(N);
	characterization_family ret(N);

	// First we accumulate the kind-of-UIOs and the separating words we need. We will do this with a
	// breath first search. If we encouter a set of states which is not a singleton, we add
	// sequences from the matrix, locally and globally.
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

			// for each distinct pair, look up the lca in the splitting tree and add that word.
			// This gives a very bad complexity! (cubic I guess)
			// I believe this can be fixed to quadratic
			for (auto && p : node.CI) {
				for (auto && q : node.CI) {
					const auto s = p.second;
					const auto t = q.second;
					if (s <= t) continue;

					vector<bool> states(N, false);
					states[s] = states[t] = true;

					const auto root = lca(separating_sequences,
					                      [&states](auto z) -> bool { return states[z]; });
					const auto & sep_word = root.seperator;

					suffixes[s].insert(sep_word);
					suffixes[t].insert(sep_word);
				}
			}

			// Finalize the suffixes
			for (auto && p : node.CI) {
				const auto s = p.second;
				auto & current_suffixes = suffixes[s];

				// they are the same (FIXME)
				ret[s].local_suffixes = flatten(current_suffixes);
				ret[s].global_suffixes = flatten(current_suffixes);
				current_suffixes.clear();
			}

			continue;
		}

		// add some work
		for (auto && i : node.word) word.push_back(i);        // extend the word
		for (auto && c : node.children) work.push({word, c}); // and visit the children with word
	}

	return ret;
}
