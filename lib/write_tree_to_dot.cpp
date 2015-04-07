#include "write_tree_to_dot.hpp"
#include "adaptive_distinguishing_sequence.hpp"
#include "splitting_tree.hpp"

#include <fstream>

using namespace std;

template <typename T>
ostream & operator<<(ostream& out, vector<T> const & x){
	if(x.empty()) return out;

	auto it = begin(x);
	out << *it++;
	while(it != end(x)) out << " " << *it++;
	return out;
}


void write_splitting_tree_to_dot(const splitting_tree& root, ostream& out_){
	write_tree_to_dot(root, [](const splitting_tree & node, ostream& out){
		out << node.states;
		if(!node.seperator.empty()){
			out << "\\n" << node.seperator;
		}
	}, out_);
}

void write_splitting_tree_to_dot(const splitting_tree& root, const string& filename){
	ofstream file(filename);
	write_splitting_tree_to_dot(root, file);
}

void write_adaptive_distinguishing_sequence_to_dot(const adaptive_distinguishing_sequence & root, ostream & out_){
	write_tree_to_dot(root, [](const adaptive_distinguishing_sequence & node, ostream& out){
		if(!node.word.empty()){
			out << node.word;
		} else {
			vector<state> I(node.CI.size());
			transform(begin(node.CI), end(node.CI), begin(I), [](const pair<state, state> p){ return p.second; });
			out << "I = " << I;
		}
	}, out_);
}

void write_adaptive_distinguishing_sequence_to_dot(const adaptive_distinguishing_sequence & root, string const & filename){
	ofstream file(filename);
	write_adaptive_distinguishing_sequence_to_dot(root, file);
}
