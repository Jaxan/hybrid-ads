#include <read_mealy_from_dot.hpp>
#include <mealy.hpp>
#include <transfer_sequences.hpp>

#include <cassert>
#include <string>
#include <fstream>
#include <iostream>
#include <queue>
#include <vector>

using namespace std;

int main(int argc, char *argv[]){
	if(argc != 3) return 1;

	const string hypo_filename = argv[1];
	const string mach_filename = argv[2];

	translation t;

	const auto hypothesis = read_mealy_from_dot(hypo_filename, t);
	const auto machine = read_mealy_from_dot(mach_filename, t);

	auto input_to_string = create_reverse_map(t.input_indices);
	auto output_to_string = create_reverse_map(t.output_indices);

	vector<bool> visited(machine.graph_size, false);
	{
		const auto state_cover = create_transfer_sequences(hypothesis, 0);
		for(auto && p : state_cover){
			state s = 0;
			for(auto && i : p){
				visited[s.base()] = true;
				s = apply(machine, s, i).to;
			}
			visited[s.base()] = true;
		}
	}

	{
		ofstream out("gephi.dot");
		out << "digraph g {\n";

		for(state s = 0; s < machine.graph_size; ++s){
			out << "\t" << "s" << s << " [";
			out << "visited=\"" << (visited[s.base()] ? "1" : "0") << "\"";
			out << "]\n";
		}

		for(state s = 0; s < machine.graph_size; ++s){
			for(input i = 0; i < machine.input_size; ++i){
				const auto ret = apply(machine, s, i);
				out << "\t" << "s" << s << " -> " << "s" << ret.to << " [";
				out << "input=\"" << input_to_string[i.base()] << "\", ";
				out << "output=\"" << output_to_string[ret.output.base()] << "\"";
				out << "]\n";
			}
		}

		out << "}" << endl;
	}
}

