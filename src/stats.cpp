#include <mealy.hpp>
#include <read_mealy.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>

using namespace std;

static void print_stats_for_machine(string filename){
	const auto machine = [&]{
		if (filename.find(".txt") != string::npos) {
			return read_mealy_from_txt(filename);
		} else if (filename.find(".dot") != string::npos) {
			return read_mealy_from_dot(filename).first;
		}

		clog << "warning: unrecognized file format, assuming dot";
		return read_mealy_from_dot(filename).first;
	}();

	cout << "machine " << filename << " has\n";
	cout << '\t' << machine.graph_size << " states\n";
	cout << '\t' << machine.input_size << " inputs\n";
	cout << '\t' << machine.output_size << " outputs\n";
}

int main(int argc, char *argv[]){
	if(argc != 2) {
		cerr << "usages: stats <filename>" << endl;
		return 1;
	}

	const string filename = argv[1];
	print_stats_for_machine(filename);
}
