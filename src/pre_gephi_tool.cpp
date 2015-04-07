#include <mealy.hpp>
#include <read_mealy_from_dot.hpp>
#include <transfer_sequences.hpp>

#include <boost/optional.hpp>

#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

using namespace std;
using namespace boost;

int main(int argc, char *argv[]){
	if(argc != 5) return argc;

	// Program options
	const string machince_filename = argv[1];
	const string machince_positions_filename = argv[2];
	const string hypo_filename_pattern = argv[3];
	const size_t maximal_hypothesis = stoul(argv[4]);

	// Read all the hypothesis
	translation t;
	vector<mealy> hypotheses;
	for(size_t i = 0; i <= maximal_hypothesis; ++i){
		clog << "Reading hypo " << i << endl;
		string hypothesis_filename = hypo_filename_pattern;
		auto it = hypothesis_filename.find('%');
		hypothesis_filename.replace(it, 1, to_string(i));
		hypotheses.push_back(read_mealy_from_dot(hypothesis_filename, t));
	}

	const auto machine = read_mealy_from_dot(machince_filename, t);

	auto input_to_string = create_reverse_map(t.input_indices);
	auto output_to_string = create_reverse_map(t.output_indices);

	// Read the positions by gephi, indexed by state
	// (export to .net file and then `tail +2 Untitled.net | awk '{print $3, $4}' > positions.txt`)
	vector<pair<double, double>> positions(machine.graph_size);
	ifstream position_file(machince_positions_filename);
	for(auto & p : positions){
		position_file >> p.first >> p.second;
	}

	// Visit all states and record in which hypo it is reached
	vector<optional<size_t>> visited(machine.graph_size);
	for(size_t i = 0; i <= maximal_hypothesis; ++i){
		clog << "Visiting hypo " << i << endl;
		const auto state_cover = create_transfer_sequences(hypotheses[i], 0);
		for(auto && p : state_cover){
			state s = 0;
			for(auto && inp : p){
				if(!visited[s]) visited[s] = i;
				s = apply(machine, s, inp).to;
			}
			if(!visited[s]) visited[s] = i;
		}
	}

	// Output a dot per hypo, making a movie
	for(size_t i = 0; i < hypotheses.size(); ++i){
		clog << "Saving frame " << i << endl;
		string hypothesis_filename = hypo_filename_pattern + ".movie";
		auto it = hypothesis_filename.find('%');
		hypothesis_filename.replace(it, 1, to_string(i));

		ofstream out(hypothesis_filename);
		out << "digraph {\n";

		for(state s = 0; s < machine.graph_size; ++s){
			bool is_visited = visited[s] ? (visited[s].value() <= i) : false;
			out << "\t" << "s" << s << " [";
			out << "color=\"" << (is_visited ? "green" : "red") << "\"" << ", ";
			out << "pos=\"" << positions[s].first << "," << positions[s].second << "\"";
			out << "]\n";
		}

		for(state s = 0; s < machine.graph_size; ++s){
			vector<bool> visited(machine.graph_size, false);
			visited[s] = true;
			for(input i = 0; i < machine.input_size; ++i){
				const auto t = apply(machine, s, i).to;
				if(visited[t]) continue;
				out << "\t" << "s" << s << " -> " << "s" << t << "\n";
				visited[t] = true;
			}
		}

		out << "}" << endl;
	}
}
