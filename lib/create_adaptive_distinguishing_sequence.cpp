#include "create_adaptive_distinguishing_sequence.hpp"
#include "splitting_tree.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <queue>
#include <vector>

using namespace std;

result2 create_adaptive_distinguishing_sequence(const result & splitting_tree){
	const auto & root = splitting_tree.root;
	const auto & succession = splitting_tree.successor_cache;
	const auto N = root.states.size();

	result2 r(N);
	auto & root_seq = r.sequence;

	{
		queue<reference_wrapper<dist_seq>> work2;
		work2.push(root_seq);

		while(!work2.empty()){
			dist_seq & node = work2.front();
			work2.pop();

			if(node.CI.size() < 2) continue;

			vector<bool> states(N, false);
			for(auto && state : node.CI){
				states[state.first.base()] = true;
			}

			const auto & oboom = lca(root, [&states](state state) -> bool{
				return states[state.base()];
			});

			if(oboom.children.empty()) continue;

			node.word = oboom.seperator;
			for(auto && c : oboom.children){
				dist_seq new_c(0, node.depth + 1);

				size_t i = 0;
				size_t j = 0;

				while(i < node.CI.size() && j < c.states.size()){
					if(node.CI[i].first < c.states[j]) {
						i++;
					} else if(node.CI[i].first > c.states[j]) {
						j++;
					} else {
						const auto curr = succession[oboom.depth][node.CI[i].first.base()];
						const auto init = node.CI[i].second;
						new_c.CI.push_back({curr, init});
						i++;
						j++;
					}
				}

				// FIXME: this should/could be done without sorting...
				sort(begin(new_c.CI), end(new_c.CI));

				if(!new_c.CI.empty()){
					node.children.push_back(move(new_c));
				}
			}

			assert(node.children.size() > 1);

			for(auto & c : node.children) {
				work2.push(c);
			}
		}
	}

	return r;
}
