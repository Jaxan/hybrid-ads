#include "seperating_family.hpp"

#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

#include <functional>
#include <stack>
#include <utility>

using namespace std;

seperating_family create_seperating_family(const adaptive_distinguishing_sequence & sequence, const seperating_matrix & all_pair_seperating_sequences){
	seperating_family seperating_family(all_pair_seperating_sequences.size());

	stack<pair<word, reference_wrapper<const adaptive_distinguishing_sequence>>> work;
	work.push({{}, sequence});

	while(!work.empty()){
		auto word = work.top().first;
		const adaptive_distinguishing_sequence & node = work.top().second;
		work.pop();

		if(node.children.empty()){
			// add sequence to this leave
			for(auto && p : node.CI){
				const auto state = p.second;
				seperating_family[state.base()].push_back(word);
			}

			// if the leaf is not a singleton, we need the all_pair seperating seqs
			for(auto && p : node.CI){
				for(auto && q : node.CI){
					const auto s = p.second;
					const auto t = q.second;
					if(s == t) continue;
					seperating_family[s.base()].push_back(all_pair_seperating_sequences[s.base()][t.base()]);
				}
			}

			continue;
		}

		for(auto && i : node.word)
			word.push_back(i);

		for(auto && c : node.children)
			work.push({word, c});
	}

	// Remove duplicates
	for(auto & vec : seperating_family){
		boost::erase(vec, boost::unique<boost::return_found_end>(boost::sort(vec)));
	}

	return seperating_family;
}
