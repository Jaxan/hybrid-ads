#include "read_mealy_from_txt.hpp"
#include "mealy.hpp"

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

#include <iostream>

using namespace std;

mealy read_mealy_from_txt(std::istream & in) {
	mealy m;

	state max_state = 0;
	input max_input = 0;
	output max_output = 0;

	string line;
	while (getline(in, line)) {
		state from, to;
		input i;
		output o;
		string seperator;

		stringstream ss(line);
		ss >> from >> seperator >> i >> seperator >> o >> seperator >> to;

		if (from >= max_state) max_state = from + 1;
		if (to >= max_state) max_state = to + 1;
		if (i >= max_input) max_input = i + 1;
		if (o >= max_output) max_output = o + 1;

		if (defined(m, from, i)) throw runtime_error("Nondeterministic machine");

		m.graph.resize(max_state);
		auto & v = m.graph[from];
		v.resize(max_input);
		v[i] = mealy::edge(to, o);

		assert(defined(m, to, i));
	}

	m.graph_size = max_state;
	m.input_size = max_input;
	m.output_size = max_output;

	if (m.graph_size == 0) throw runtime_error("Empty state set");
	if (m.input_size == 0) throw runtime_error("Empty input set");
	if (m.output_size == 0) throw runtime_error("Empty output set");
	if (!is_complete(m)) throw runtime_error("Partial machine");
	return m;
}

mealy read_mealy_from_txt(const std::string & filename) {
	std::ifstream file(filename);
	return read_mealy_from_txt(file);
}


translation create_translation_for_mealy(const mealy & m) {
	translation t;
	t.max_input = m.input_size;
	t.max_output = m.output_size;

	for (input i = 0; i < t.max_input; ++i) {
		t.input_indices[to_string(i)] = i;
	}

	for (output o = 0; o < t.max_output; ++o) {
		t.output_indices[to_string(o)] = o;
	}

	return t;
}
