#include <mealy.hpp>
#include <partition.hpp>
#include <read_mealy_from_dot.hpp>
#include <splitting_tree.hpp>
#include <write_splitting_tree_to_dot.hpp>

#include <cassert>
#include <numeric>
#include <queue>
#include <type_traits>
#include <vector>

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
	const auto push = [&work](auto br, auto & sp){ work.push({br, sp}); };
	const auto pop = [&work](){ const auto r = work.front(); work.pop(); return r; };

	push(part.find(0), root);

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
		// if(elems_in(*block) == 1) continue;

		if(verbose){
			cout << "considering" << endl;
		}

		// First try to split on output
		for(input symbol = 0; symbol < P; ++symbol){
			auto new_blocks = part.refine(block, [symbol, &g](state state){
				return apply(g, state, symbol).output.base();
			}, Q);

			if(elems_in(new_blocks) == 1){
				// continue with other input symbols
				// (refine always updates the block)
				block = new_blocks.first;
				continue;
			}

			// a succesful split, add the children
			const auto nb = distance(new_blocks.first, new_blocks.second);
			boom.children.assign(nb, splijtboom(0));
			boom.seperator = {symbol};

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

			if(oboom.children.empty()){
				// a leaf, hence not a split, try other symbols
				continue;
			}

			if(verbose){
				cout << "split\t";
				for(auto s : oboom.states) cout << s << " ";
				cout << endl;
				cout << "into ";
				for(auto & c : oboom.children) {
					for(auto s : c.states) cout << s << " ";
					cout << "- ";
				}
				cout << endl;
			}

			// a succesful split, construct the children
			boom.seperator.resize(oboom.seperator.size() + 1);
			auto it = begin(boom.seperator);
			*it++ = symbol;
			copy(begin(oboom.seperator), end(oboom.seperator), it);

			auto new_blocks = part.refine(block, [&boom, &g](size_t state){
				return apply(g, state, begin(boom.seperator), end(boom.seperator)).output.base();
			}, Q);

			if(elems_in(new_blocks) == 1){
				throw logic_error("Refinement did not give finer partition, can not happen");
			}

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
				cout << "splitted state into " << nb << endl;
			}

			goto has_split;
		}
		push(block, boom);
		cout << "no split :(" << endl;

		has_split:
		cout << "we have " << part.size() << " blocks / " << N << " states" << endl;
		cout << "and still " << work.size() << " work" << endl;
	}

	write_splitting_tree_to_dot(root, filename + ".splitting_tree.dot");
}

