#include "read_mealy_from_dot.hpp"
#include "mealy.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

template <typename T>
T get(istream& in){
	T t;
	in >> t;
	return t;
}

mealy read_mealy_from_dot(std::istream & in, translation & t){
	mealy m;

	std::map<std::string, state> state_indices;
	state max_state = 0;

	string line;
	stringstream ss;
	while(getline(in, line)){
		if(line.find("}") != string::npos) break;

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

		// make fresh indices, if needed
		if(state_indices.count(lh) < 1) state_indices[lh] = max_state++;
		if(state_indices.count(rh) < 1) state_indices[rh] = max_state++;
		if(t.input_indices.count(input) < 1) t.input_indices[input] = t.max_input++;
		if(t.output_indices.count(output) < 1) t.output_indices[output] = t.max_output++;

		// add edge
		m.graph.resize(max_state);
		auto & v = m.graph[state_indices[lh]];
		v.resize(t.max_input);
		v[t.input_indices[input]] = mealy::edge(state_indices[rh], t.output_indices[output]);
	}

	m.graph_size = max_state;
	m.input_size = t.max_input;
	m.output_size = t.max_output;

	if(m.graph_size == 0) throw runtime_error("Empty state set");
	if(m.input_size == 0) throw runtime_error("Empty input set");
	if(m.output_size == 0) throw runtime_error("Empty output set");
	if(!is_complete(m)) throw runtime_error("Partial machine");
	return m;
}


mealy read_mealy_from_dot(const string & filename, translation & t){
	ifstream file(filename);
	return read_mealy_from_dot(file, t);
}


std::pair<mealy, translation> read_mealy_from_dot(istream & in){
	translation t;
	const auto m = read_mealy_from_dot(in, t);
	return {move(m), move(t)};
}


std::pair<mealy, translation> read_mealy_from_dot(const string & filename){
	translation t;
	const auto m = read_mealy_from_dot(filename, t);
	return {move(m), move(t)};
}
