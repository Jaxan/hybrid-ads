#include "transfer_sequences.hpp"

#include "mealy.hpp"

#include <queue>

using namespace std;

transfer_sequences create_transfer_sequences(const mealy& machine, state s){
	vector<bool> visited(machine.graph_size, false);
	vector<word> words(machine.graph_size);

	queue<state> work;
	work.push(s);
	while(!work.empty()){
		const auto u = work.front();
		work.pop();

		if(visited[u.base()]) continue;

		visited[u.base()] = true;

		for(input i = 0; i < machine.input_size; ++i){
			const auto v = apply(machine, u, i).to;
			if(visited[v.base()]) continue;

			words[v.base()] = words[u.base()];
			words[v.base()].push_back(i);
			work.push(v);
		}
	}

	return words;
}

std::vector<transfer_sequences> create_all_transfer_sequences(const mealy& machine){
	vector<transfer_sequences> transfer_sequences(machine.graph_size);
	for(state s = 0; s < machine.graph_size; ++s){
		transfer_sequences[s.base()] = create_transfer_sequences(machine, s);
	}
	return transfer_sequences;
}
