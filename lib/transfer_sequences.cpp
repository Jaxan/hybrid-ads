#include "transfer_sequences.hpp"

#include "mealy.hpp"

#include <algorithm>
#include <numeric>
#include <queue>
#include <random>
#include <tuple>

using namespace std;

transfer_sequences create_transfer_sequences(transfer_options const & opt, const mealy & machine,
                                             state s, uint_fast32_t random_seed) {
	mt19937 generator(random_seed);

	vector<bool> added(machine.graph_size, false);
	vector<word> words(machine.graph_size);
	vector<input> all_inputs(machine.input_size);
	iota(begin(all_inputs), end(all_inputs), input(0));

	// state
	queue<state> work;
	work.push(s);
	added[s] = true;
	while (!work.empty()) {
		const auto u = work.front();
		work.pop();

		// NOTE: we could also shuffle work, but we would need to do this per distance
		// the current shuffle is an approximation of real randomization, but easier to implement.
		if (opt.randomized) shuffle(begin(all_inputs), end(all_inputs), generator);
		for (input i : all_inputs) {
			const auto v = apply(machine, u, i).to;
			if (added[v]) continue;

			work.push(v);
			added[v] = true;
			words[v] = words[u];
			words[v].push_back(i);
		}
	}

	return words;
}
