#include <read_mealy_from_dot.hpp>
#include <mealy.hpp>

#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]){
	if(argc != 2) return 37;

	const auto filename = argv[1];
	ifstream file(filename);
	const auto machine = read_mealy_from_dot(file);

	cout << "machine " << filename << " has\n";
	cout << '\t' << machine.graph_size << " states\n";
	cout << '\t' << machine.input_size << " inputs\n";
	cout << '\t' << machine.output_size << " outputs\n";
}

