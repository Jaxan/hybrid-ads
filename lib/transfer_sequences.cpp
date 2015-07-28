#include "transfer_sequences.hpp"

#include "mealy.hpp"

#include <algorithm>
#include <numeric>
#include <queue>
#include <random>

using namespace std;

transfer_sequences create_transfer_sequences(const mealy& machine, state s){
	vector<bool> visited(machine.graph_size, false);
	vector<word> words(machine.graph_size);

	priority_queue<pair<int, state>> work;
	work.push({0, s});
	while(!work.empty()){
		const auto p = work.top();
		const auto u = p.second;
		const auto d = p.first - 1;
		work.pop();

		if(visited[u]) continue;

		visited[u] = true;

		for(input i = 0; i < machine.input_size; ++i){
			const auto v = apply(machine, u, i).to;
			if(visited[v]) continue;

			words[v] = words[u];
			words[v].push_back(i);
			work.push({d, v});
		}
	}

	return words;
}

transfer_sequences create_randomized_transfer_sequences(const mealy & machine, state s, uint_fast32_t random_seed){
	mt19937 generator(random_seed);

	vector<bool> visited(machine.graph_size, false);
	vector<word> words(machine.graph_size);
	vector<input> all_inputs(machine.input_size);
	iota(begin(all_inputs), end(all_inputs), input(0));

	priority_queue<pair<int, state>> work;
	work.push({0, s});
	while(!work.empty()){
		const auto p = work.top();
		const auto u = p.second;
		const auto d = p.first - 1;
		work.pop();

		if(visited[u]) continue;

		visited[u] = true;

		shuffle(begin(all_inputs), end(all_inputs), generator);
		for(input i : all_inputs){
			const auto v = apply(machine, u, i).to;
			if(visited[v]) continue;

			words[v] = words[u];
			words[v].push_back(i);
			work.push({d, v});
		}
	}

	return words;
}
