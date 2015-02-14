#include <partition.hpp>
#include <mealy.hpp>
#include <read_mealy_from_dot.hpp>

#include <cassert>
#include <queue>
#include <vector>
#include <type_traits>

using namespace std;

int verbose = 0;

// TODO: I was working on this...
// I didn't know how to do lca for this case...
struct splijtboom {
	vector<size_t> states;
	vector<splijtboom> children;

	const splijtboom * parent = nullptr;
};

template <typename Fun>
bool lca_impl(const splijtboom & node, Fun && f){
	int count = 0;
	for(auto && c : node.children){
		if(lca_impl(c, f)) count++;
	}
	if(count >= 2) return true;
}

template <typename Fun>
const splijtboom & lca(const splijtboom & root, Fun && f){
	static_assert(is_same<decltype(f(0)), bool>::value, "f should return a bool");
	lca_impl(root, f);
}

int main(int argc, char *argv[]){
	if(argc < 2) return 1;
	if(argc > 2) verbose = argc - 2;

	const auto g = read_mealy_from_dot(argv[1], verbose);
	assert(is_complete(g));

	const auto N = g.graph.size();
	const auto P = g.input_indices.size();
	const auto Q = g.output_indices.size();

	partition_refine part(N);
	cout << "starting with " << part.size() << " blocks / " << N << " states" << endl;

	queue<partition_refine::BlockRef> work;
	const auto push = [&work](auto br){ work.push(br); };
	const auto pop = [&work](){ const auto r = work.front(); work.pop(); return r; };

	push(part.find(0));

	while(!work.empty()){
		auto block = pop();
		if(elems_in(*block) == 1) continue;

		bool splitted = false;
		for(int symbol = 0; symbol < P; ++symbol){
			auto new_blocks = part.refine(block, [symbol, &g](size_t state){
				return g.graph[state][symbol].output;
			}, Q);

			if(elems_in(new_blocks) == 1){
				// continue with other input symbols
				// (refine always updates the block)
				block = new_blocks.first;
				continue;
			}

			// a succesful split, add the children
			while(new_blocks.first != new_blocks.second){
				push(new_blocks.first++);
			}
			splitted = true;
			break;
		}
		if(!splitted) cout << "no split :(" << endl;
		cout << "we have " << part.size() << " blocks / " << N << " states" << endl;
	}
	cout << "jippiejeejjo" << endl;
}

