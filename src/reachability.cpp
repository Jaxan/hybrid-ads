#include <mealy.hpp>
#include <read_mealy_from_dot.hpp>

#include <fstream>
#include <iostream>
#include <queue>
#include <vector>

using namespace std;

int main(int argc, char *argv[]){
	if(argc != 3) return 1;

	const string filename = argv[1];
	const string machince_positions_filename = argv[2];

	// Read machine and its positions
	const auto machine_translation = read_mealy_from_dot(filename);
	const auto & machine = machine_translation.first;
	const auto & translation = machine_translation.second;

	vector<pair<double, double>> positions(machine.graph_size);
	ifstream position_file(machince_positions_filename);
	for(auto & p : positions){
		position_file >> p.first >> p.second;
	}

	// read subalphabet
	cout << "Machine is read, please provide a subalphabet" << endl;
	vector<input> subalphabet;
	string i;
	while(cin >> i){
		const input x = translation.input_indices.at(i);
		subalphabet.push_back(x);
	}

	// visit with subalphabet
	vector<bool> visited(machine.graph_size, false);
	queue<state> work;
	work.push(0);
	while(!work.empty()){
		const state s = work.front();
		work.pop();

		if(visited[s.base()]) continue;
		visited[s.base()] = true;

		for(auto x : subalphabet){
			const state t = apply(machine, s, x).to;
			if(!visited[t.base()]) work.push(t);
		}
	}

	// write to dot
	ofstream out(filename + ".reachable.dot");
	out << "digraph {\n";

	for(state s = 0; s < machine.graph_size; ++s){
		bool is_visited = visited[s.base()];
		out << "\t" << "s" << s << " [";
		out << "color=\"" << (is_visited ? "green" : "red") << "\"" << ", ";
		out << "pos=\"" << positions[s.base()].first << "," << positions[s.base()].second << "\"";
		out << "]\n";
	}

	for(state s = 0; s < machine.graph_size; ++s){
		vector<bool> visited(machine.graph_size, false);
		visited[s.base()] = true;
		for(input i = 0; i < machine.input_size; ++i){
			const auto t = apply(machine, s, i).to;
			if(visited[t.base()]) continue;
			out << "\t" << "s" << s << " -> " << "s" << t << "\n";
			visited[t.base()] = true;
		}
	}

	out << "}" << endl;
}

