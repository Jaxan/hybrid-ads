#include <mealy.hpp>
#include <partition.hpp>
#include <read_mealy_from_dot.hpp>
#include <splitting_tree.hpp>
#include <write_splitting_tree_to_dot.hpp>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <queue>
#include <utility>

using namespace std;

template <typename T>
ostream & operator<<(ostream& out, vector<T> const & x){
	if(x.empty()) return out;

	auto it = begin(x);
	out << *it++;
	while(it != end(x)) out << " " << *it++;
	return out;
}

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

	cerr << "* Reading file " << filename << "\n";
	const auto g = read_mealy_from_dot(filename, verbose);
	assert(is_complete(g));
	cerr << "\tdone\n";

	const auto N = g.graph.size();
	const auto P = g.input_indices.size();
	const auto Q = g.output_indices.size();

	cerr << "* Setting up strucutres\n";
	partition_refine part(N);
	splijtboom root(N, 0);
	vector<vector<state>> succession;

	queue<pair<partition_refine::BlockRef, reference_wrapper<splijtboom>>> work;
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
	const auto update_succession = [N, &succession](state s, state t, size_t depth){
		if(succession.size() < depth+1) succession.resize(depth+1, vector<state>(N, -1));
		succession[depth][s.base()] = t;
	};

	push(part.begin(), root);
	cerr << "\tdone\n";

	size_t days_without_progress = 0;
	string filename_thingy;

	cerr << "* Starting Lee & Yannakakis I\n";
	while(!work.empty()){
		const auto block_boom = pop();
		const auto block = block_boom.first;
		splijtboom & boom = block_boom.second;
		const auto depth = boom.depth;

		if(verbose){
			cout << "current\t";
			for(auto s : boom.states) cout << s << " ";
			cout << endl;
		}

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
				// cerr << "WARNING: Refinement did not give finer partition, can not happen\n";
				continue;
			}

			// update partition and add the children
			boom.seperator = word;
			const auto range = part.replace(block, move(new_blocks));
			add_push_new_block(range, boom);

			goto has_split;
		}

		// cout << "no split :(" << endl;
		if(days_without_progress++ >= work.size()) {
			filename_thingy = "incomplete_";
			cerr << "\t* No distinguishing seq found!\n";
			break;
		}
		push(block, boom);
		continue;

		has_split:
		// cout << "blocks: " << part.size() << ", states: " << N << ", work: " << work.size() << endl;
		days_without_progress = 0;
	}
	cerr << "\tdone\n";

	cerr << "* Write splitting tree\n";
	write_splitting_tree_to_dot(root, filename + "." + filename_thingy + "splitting_tree");
	cerr << "\tdone\n";

	struct dist_seq {
		dist_seq(size_t N)
		: I(N)
		, C(I)
		{
			iota(begin(I), end(I), 0);
			iota(begin(C), end(C), 0);
		}

		vector<state> I;
		vector<state> C;
		vector<input> word;
		vector<dist_seq> children;
	} root_seq(N);

	cerr << "* Lee and Yannakaki II\n";
	{
		queue<reference_wrapper<dist_seq>> work2;
		work2.push(root_seq);

		while(!work2.empty()){
			dist_seq & node = work2.front();
			work2.pop();

			if(node.C.size() < 2) continue;

			vector<bool> states(N, false);
			for(auto && state : node.C){
				states[state.base()] = true;
			}

			const auto & oboom = lca(root, [&states](state state) -> bool{
				return states[state.base()];
			});

			if(oboom.children.empty()) continue;

			node.word = oboom.seperator;
			for(auto && c : oboom.children){
				dist_seq new_c(0);

				size_t i = 0;
				size_t j = 0;

				while(i < node.C.size() && j < c.states.size()){
					if(node.C[i] < c.states[j]) {
						i++;
					} else if(node.C[i] > c.states[j]) {
						j++;
					} else {
						new_c.I.push_back(node.I[i]);
						new_c.C.push_back(succession[oboom.depth][node.C[i].base()]);
						i++;
						j++;
					}
				}

				// woops. fixme
				sort(begin(new_c.C), end(new_c.C));

				if(!new_c.C.empty()){
					node.children.push_back(move(new_c));
				}
			}

			for(auto & c : node.children) {
				work2.push(c);
			}
		}
	}
	cerr << "\tdone\n";

	cerr << "* Write dist sequence\n";
	{
		ofstream out(filename + "." + filename_thingy + "dist_seq");
		out << "digraph distinguishing_sequence {\n";

		// breadth first
		int global_id = 0;
		queue<pair<int, reference_wrapper<const dist_seq>>> work3;
		work3.push({global_id++, root_seq});
		while(!work3.empty()){
			const auto id = work3.front().first;
			const dist_seq & node = work3.front().second;
			work3.pop();

			if(!node.word.empty()){
				out << "\n\ts" << id << " [label=\"" << node.word << "\"];\n";
			} else {
				out << "\n\ts" << id << " [label=\"I = " << node.I << "\"];\n";
			}

			for(auto && c : node.children){
				int new_id = global_id++;
				out << "\ts" << id << " -> " << "s" << new_id << ";\n";
				work3.push({new_id, c});
			}
		}

		out << "}" << endl;
	}
	cerr << "\tdone\n" << endl;
}

