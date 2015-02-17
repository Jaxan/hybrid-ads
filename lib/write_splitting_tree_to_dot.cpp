#include "write_splitting_tree_to_dot.hpp"
#include "splitting_tree.hpp"

#include <fstream>
#include <functional>
#include <ostream>
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

void write_splitting_tree_to_dot(const splijtboom& root, ostream& out){
	out << "digraph splijtboom {\n";

	// breadth first
	int global_id = 0;
	queue<pair<int, reference_wrapper<const splijtboom>>> work;
	work.push({global_id++, root});
	while(!work.empty()){
		const auto id = work.front().first;
		const splijtboom & node = work.front().second;
		work.pop();

		out << "\n\ts" << id << " [label=\"" << node.states;
		if(!node.seperator.empty()) out << "\\n" << node.seperator;
		out << "\"];\n";

		for(auto && c : node.children){
			int new_id = global_id++;
			out << "\ts" << id << " -> " << "s" << new_id << ";\n";
			work.push({new_id, c});
		}
	}

	out << "}" << endl;
}


void write_splitting_tree_to_dot(const splijtboom& root, const string& filename){
	ofstream file(filename);
	write_splitting_tree_to_dot(root, file);
}
