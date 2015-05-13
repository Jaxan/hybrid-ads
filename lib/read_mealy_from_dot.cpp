#include "read_mealy_from_dot.hpp"
#include "mealy.hpp"

#include <boost/algorithm/string/trim.hpp>

#include <fstream>
#include <stdexcept>
#include <string>

using namespace std;

static string easy_substr(string const & s, size_t begin, size_t end){
	return s.substr(begin, end - begin);
}

mealy read_mealy_from_dot(std::istream & in, translation & t){
	mealy m;

	std::map<std::string, state> state_indices;
	state max_state = 0;

	string line;
	while(getline(in, line)){
		using boost::algorithm::trim_copy;
		const auto npos = std::string::npos;

		if(line.find("}") != string::npos) break;

		// parse states
		const auto arrow_pos = line.find("->");
		const auto bracket_pos = line.find('[');
		if(arrow_pos == npos || bracket_pos == npos) continue;

		const auto lh = trim_copy(easy_substr(line, 0, arrow_pos));
		const auto rh = trim_copy(easy_substr(line, arrow_pos+2, bracket_pos));

		// parse input/output
		const auto quote1_pos = line.find('\"', bracket_pos);
		const auto slash_pos = line.find('/', quote1_pos);
		const auto quote2_pos = line.find('\"', slash_pos);
		if(quote1_pos == npos || slash_pos == npos || quote2_pos == npos) continue;

		const auto input = trim_copy(easy_substr(line, quote1_pos+1, slash_pos));
		const auto output = trim_copy(easy_substr(line, slash_pos+1, quote2_pos));

		// make fresh indices, if needed
		if(state_indices.count(lh) < 1) state_indices[lh] = max_state++;
		if(state_indices.count(rh) < 1) state_indices[rh] = max_state++;
		if(t.input_indices.count(input) < 1) t.input_indices[input] = t.max_input++;
		if(t.output_indices.count(output) < 1) t.output_indices[output] = t.max_output++;

		if(defined(m, state_indices[lh], t.input_indices[input]))
			throw runtime_error("Nondeterministic machine");

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
