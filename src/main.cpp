#include <create_adaptive_distinguishing_sequence.hpp>
#include <create_splitting_tree.hpp>
#include <read_mealy_from_dot.hpp>
#include <write_tree_to_dot.hpp>

#include <cassert>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]){
	if(argc != 2) return 1;
	const string filename = argv[1];

	cerr << "* Reading file " << filename << "\n";
	const auto machine = read_mealy_from_dot(filename);
	assert(is_complete(machine));
	cerr << "\tdone\n";

	cerr << "* Starting Lee & Yannakakis I\n";
	const auto splitting_tree = create_splitting_tree(machine);
	cerr << "\tdone\n";

	cerr << "* Write splitting tree\n";
	const string tree_filename = splitting_tree.is_complete ? (filename + ".splitting_tree") : (filename + ".incomplete_splitting_tree");
	write_splitting_tree_to_dot(splitting_tree.root, tree_filename);
	cerr << "\tdone\n";

	cerr << "* Lee and Yannakaki II\n";
	const auto distinguishing_sequence = create_adaptive_distinguishing_sequence(splitting_tree);
	cerr << "\tdone\n";

	cerr << "* Write dist sequence\n";
	const string dseq_filename = splitting_tree.is_complete ? (filename + ".dist_seq") : (filename + ".incomplete_dist_seq");
	write_adaptive_distinguishing_sequence_to_dot(distinguishing_sequence.sequence, dseq_filename);
	cerr << "\tdone\n" << endl;
}

