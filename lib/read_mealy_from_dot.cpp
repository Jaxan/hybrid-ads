#include "read_mealy_from_dot.hpp"

#include "mealy.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

template <typename T>
T get(istream& in){
	T t;
	in >> t;
	return t;
}

Mealy read_mealy_from_dot(istream& in, int verbose){
	Mealy m;

	string line;
	stringstream ss;
	while(getline(in, line)){
		const auto i = line.find("->");
		if(i == string::npos) continue;

		// get from and to state
		ss.str(line);
		ss.seekg(0);
		const auto lh = get<string>(ss);
		const auto arrow = get<string>(ss);
		const auto rh = get<string>(ss);

		// get label
		const auto l1 = line.find('\"');
		const auto l2 = line.find('\"', l1+1);
		if(l1 == string::npos || l2 == string::npos) continue;
		ss.str(line.substr(l1+1, l2-(l1+1)));
		ss.seekg(0);

		const auto input = get<string>(ss);
		const auto slash = get<string>(ss);
		const auto output = get<string>(ss);

		if(verbose >= 2){
			cout << lh << '\t' << rh << '\t' << input << '\t' << output << endl;
		}

		// make fresh indices, if needed
		if(m.nodes_indices.count(lh) < 1) m.nodes_indices[lh] = m.graph_size++;
		if(m.nodes_indices.count(rh) < 1) m.nodes_indices[rh] = m.graph_size++;
		if(m.input_indices.count(input) < 1) m.input_indices[input] = m.input_size++;
		if(m.output_indices.count(output) < 1) m.output_indices[output] = m.output_size++;

		// add edge
		m.graph.resize(m.graph_size);
		auto & v = m.graph[m.nodes_indices[lh].base()];
		v.resize(m.input_size);
		v[m.input_indices[input].base()] = {m.nodes_indices[rh], m.output_indices[output]};
	}

	if(verbose >= 1){
		cout << "input_alphabet = \n";
		for(auto && i : m.input_indices) cout << i.first << " ";
		cout << endl;

		cout << "output_alphabet = \n";
		for(auto && o : m.output_indices) cout << o.first << " ";
		cout << endl;
	}

	return m;
}

Mealy read_mealy_from_dot(const string& filename, int verbose){
	ifstream file(filename);
	return read_mealy_from_dot(file, verbose);
}
