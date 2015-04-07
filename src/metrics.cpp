#include <mealy.hpp>
#include <read_mealy_from_dot.hpp>

#include <iostream>
#include <vector>
#include <queue>

using namespace std;

static vector<bool> create_transfer_sequences(const mealy& machine, const state s, const input ignore){
	vector<bool> visited(machine.graph_size, false);

	queue<state> work;
	work.push(s);
	while(!work.empty()){
		const auto u = work.front();
		work.pop();

		if(visited[u]) continue;
		visited[u] = true;

		for(input i = 0; i < machine.input_size; ++i){
			if(i == ignore) continue;
			const auto v = apply(machine, u, i).to;
			if(visited[v]) continue;
			work.push(v);
		}
	}

	return visited;
}

int main(int argc, char *argv[]){
	if(argc != 2) return 1;
	const string filename = argv[1];

	const auto result = read_mealy_from_dot(filename);
	const auto & machine = result.first;
	const auto & translation = result.second;

//	vector<vector<bool>> table(machine.input_size);
//	for(input i = 0; i < machine.input_size; ++i){
//		table[i] = create_transfer_sequences(machine, 0, i);
//	}

	// note the wrong iteration ;D
	for(state s = 0; s < machine.graph_size; ++s){
		size_t scores[3] = {0, 0, 0};
		for(input i = 0; i < machine.input_size; ++i){
			const auto test1 = apply(machine, s, i).output != translation.output_indices.at("quiescence");
			const auto test2 = apply(machine, s, i).to != s;

			scores[test1 + test2]++;
		}
		cout << scores[2] << " " << scores[1] << " " << scores[0] << endl;
	}
}

