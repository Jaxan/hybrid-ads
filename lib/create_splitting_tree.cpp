#include "create_splitting_tree.hpp"
#include "logging.hpp"

#include <functional>
#include <iostream>
#include <queue>
#include <utility>

using namespace std;

template <typename T>
std::vector<T> concat(std::vector<T> const & l, std::vector<T> const & r){
	std::vector<T> ret(l.size() + r.size());
	auto it = copy(begin(l), end(l), begin(ret));
	copy(begin(r), end(r), it);
	return ret;
}

result create_splitting_tree(const Mealy& g){
	const auto N = g.graph.size();
	const auto P = g.input_indices.size();
	const auto Q = g.output_indices.size();

	result r(N);
	auto & part = r.partition;
	auto & root = r.root;
	auto & succession = r.successor_cache;

	/* We'll use a queue to keep track of leaves we have to investigate;
	 * In some cases we cannot split, and have to wait for other parts of the
	 * tree. We keep track of how many times we did no work. If this is too
	 * much, there is no complete splitting tree.
	 */
	queue<pair<partition_refine::BlockRef, reference_wrapper<splijtboom>>> work;
	size_t days_without_progress = 0;

	// Some lambda functions capturing some state, makes the code a bit easier :)
	const auto push = [&work](auto br, auto & sp) { work.push({br, sp}); };
	const auto pop = [&work]() { const auto r = work.front(); work.pop(); return r; };
	const auto add_push_new_block = [&](auto new_blocks, auto & boom) {
		const auto nb = distance(new_blocks.first, new_blocks.second);
		boom.children.assign(nb, splijtboom(0, boom.depth + 1));

		auto i = 0;
		while(new_blocks.first != new_blocks.second){
			for(auto && s : *new_blocks.first){
				boom.children[i].states.push_back(s);
			}

			push(new_blocks.first++, boom.children[i++]);
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
	const auto update_succession = [N, &succession](state s, state t, size_t depth){
		if(succession.size() < depth+1) succession.resize(depth+1, vector<state>(N, -1));
		succession[depth][s.base()] = t;
	};

	// We'll start with the root, obviously
	push(part.begin(), root);
	while(!work.empty()){
		const auto block_boom = pop();
		const auto block = block_boom.first;
		splijtboom & boom = block_boom.second;
		const auto depth = boom.depth;

		if(boom.states.size() == 1) continue;

		// First try to split on output
		for(input symbol = 0; symbol < P; ++symbol){
			const auto new_blocks = part.refine(*block, [symbol, depth, &g, &update_succession](state state){
				const auto ret = apply(g, state, symbol);
				update_succession(state, ret.to, depth);
				return ret.output.base();
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

			const auto & oboom = lca(root, [&successor_states](state state) -> bool{
				return successor_states[state.base()];
			});

			// a leaf, hence not a split -> try other symbols
			if(oboom.children.empty()) continue;

			// possibly a succesful split, construct the children
			const auto word = concat({symbol}, oboom.seperator);
			const auto new_blocks = part.refine(*block, [word, depth, &g, &update_succession](state state){
				const auto ret = apply(g, state, begin(word), end(word));
				update_succession(state, ret.to, depth);
				return ret.output.base();
			}, Q);

			// not a valid split -> continue
			if(!is_valid(new_blocks, symbol)) continue;

			if(new_blocks.size() == 1){
				fire_once([]{ cerr << "WARNING: Refinement did not give finer partition, can not happen\n"; });
				continue;
			}

			// update partition and add the children
			boom.seperator = word;
			const auto range = part.replace(block, move(new_blocks));
			add_push_new_block(range, boom);

			goto has_split;
		}

		// We tried all we could, but did not succeed => declare incompleteness.
		if(days_without_progress++ >= work.size()) {
			r.is_complete = false;
			return r;
		}
		push(block, boom);
		continue;

		has_split:
		days_without_progress = 0;
	}

	return r;
}
