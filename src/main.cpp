#include <mealy.hpp>
#include <partition.hpp>
#include <read_mealy_from_dot.hpp>
#include <splitting_tree.hpp>
#include <write_splitting_tree_to_dot.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <queue>
#include <utility>

using namespace std;

int verbose = 0;

template <typename T>
std::vector<T> concat(std::vector<T> const & l, std::vector<T> const & r){
	std::vector<T> ret(l.size() + r.size());
	auto it = copy(begin(l), end(l), begin(ret));
	copy(begin(r), end(r), it);
	return ret;
}

int main(int argc, char *argv[]){
	if(argc < 2) return 1;
	if(argc > 2) verbose = argc - 2;

	const string filename = argv[1];
	const auto g = read_mealy_from_dot(filename, verbose);
	assert(is_complete(g));

	const auto N = g.graph.size();
	const auto P = g.input_indices.size();
	const auto Q = g.output_indices.size();

	partition_refine part(N);
	splijtboom root(N);
	cout << "starting with " << part.size() << " blocks / " << N << " states" << endl;

	queue<pair<partition_refine::BlockRef, reference_wrapper<splijtboom>>> work;
	const auto push = [&work](auto br, auto & sp) { work.push({br, sp}); };
	const auto pop = [&work]() { const auto r = work.front(); work.pop(); return r; };
	const auto add_push_new_block = [&](auto new_blocks, auto & boom) {
		const auto nb = distance(new_blocks.first, new_blocks.second);
		boom.children.assign(nb, splijtboom(0));

		auto i = 0;
		while(new_blocks.first != new_blocks.second){
			for(auto && s : *new_blocks.first){
				boom.children[i].states.push_back(s);
			}

			push(new_blocks.first++, boom.children[i++]);
		}

		if(verbose){
			cout << "splitted output into " << nb << endl;
		}
	};
	const auto is_valid = [N, &g](auto blocks, auto symbol){
		for(auto && block : blocks) {
			partition_refine s_part(block);
			const auto new_blocks = s_part.refine(*s_part.begin(), [symbol, &g](state state){
				return apply(g, state, symbol).to.base();
			}, N);
			for(auto && new_block : new_blocks){
				if(distance(new_block.begin(), new_block.end()) != 1) return false;
			}
		}
		return true;
	};

	push(part.begin(), root);

	size_t days_without_progress = 0;

	while(!work.empty()){
		auto block_boom = pop();
		auto block = block_boom.first;
		splijtboom & boom = block_boom.second;

		if(verbose){
			cout << "current\t";
			for(auto s : boom.states) cout << s << " ";
			cout << endl;
		}

		if(boom.states.size() == 1) continue;

		// First try to split on output
		for(input symbol = 0; symbol < P; ++symbol){
			const auto new_blocks = part.refine(*block, [symbol, &g](state state){
				return apply(g, state, symbol).output.base();
			}, Q);

			// no split -> continue with other input symbols
			if(new_blocks.size() == 1) continue;

			// not a valid split -> continue
			if(!is_valid(new_blocks, symbol)) continue;

			// a succesful split, update partition and add the children
			boom.seperator = {symbol};
			const auto range = part.replace(block, move(new_blocks));
			add_push_new_block(range, boom);

			goto has_split;
		}

		// Then try to split on state
		for(input symbol = 0; symbol < P; ++symbol){
			vector<bool> successor_states(N, false);
			for(auto && state : *block){
				successor_states[apply(g, state, symbol).to.base()] = true;
			}

			auto & oboom = lca(root, [&successor_states](state state) -> bool{
				return successor_states[state.base()];
			});

			// a leaf, hence not a split -> try other symbols
			if(oboom.children.empty()) continue;

			// possibly a succesful split, construct the children
			const auto word = concat({symbol}, oboom.seperator);
			const auto new_blocks = part.refine(*block, [word, &g](size_t state){
				return apply(g, state, begin(word), end(word)).output.base();
			}, Q);

			// not a valid split -> continue
			if(!is_valid(new_blocks, symbol)) continue;

			if(new_blocks.size() == 1){
				cerr << "WARNING: Refinement did not give finer partition, can not happen\n";
				continue;
			}

			// update partition and add the children
			boom.seperator = word;
			const auto range = part.replace(block, move(new_blocks));
			add_push_new_block(range, boom);

			goto has_split;
		}

		cout << "no split :(" << endl;
		if(days_without_progress++ >= work.size()) {
			cerr << "No distinguishing seq found!\n";
			break;
		}
		push(block, boom);
		continue;

		has_split:
		cout << "blocks: " << part.size() << ", states: " << N << ", work: " << work.size() << endl;
		days_without_progress = 0;
	}

	write_splitting_tree_to_dot(root, filename + ".splitting_tree.dot");
}

