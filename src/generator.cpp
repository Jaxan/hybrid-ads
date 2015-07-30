#include <mealy.hpp>
#include <reachability.hpp>
#include <splitting_tree.hpp>

#include <iostream>
#include <random>
#include <fstream>

using namespace std;

static size_t number_of_leaves(splitting_tree const & root) {
	if (root.children.empty()) return 1;

	return accumulate(root.children.begin(), root.children.end(), 0ul,
	                  [](auto const & l, auto const & r) { return l + number_of_leaves(r); });
}

static mealy generate_random_machine(size_t N, size_t P, size_t Q, mt19937 & gen) {
	mealy m;

	m.graph_size = N;
	m.input_size = P;
	m.output_size = Q;

	m.graph.assign(m.graph_size, vector<mealy::edge>(m.input_size));

	uniform_int_distribution<output> o_dist(0, m.output_size - 1);
	uniform_int_distribution<state> s_dist(0, m.graph_size - 1);

	for (state s = 0; s < m.graph_size; ++s) {
		for (input i = 0; i < m.input_size; ++i) {
			m.graph[s][i] = {s_dist(gen), o_dist(gen)};
		}
	}

	return m;
}

static void print_machine(mealy const & m, size_t count) {
	ofstream file("machine_" + to_string(m.graph_size) + "_" + to_string(m.input_size) + "_" + to_string(m.output_size) + "_" + to_string(count) + ".txt");
	for (state s = 0; s < m.graph_size; ++s) {
		for (input i = 0; i < m.input_size; ++i) {
			auto e = m.graph[s][i];
			file << s << " -- " << i << " / " << e.out << " -> " << e.to << endl;
		}
	}
}

int main(int argc, char * argv[]) {
	if (argc != 5 && argc != 6) {
		cerr << "usage: generator <N> <P> <Q> <number of machines> [<seed>]" << endl;
		return 1;
	}

	const auto N = stoul(argv[1]);
	const auto P = stoul(argv[2]);
	const auto Q = stoul(argv[3]);
	const auto number_of_machines = stoul(argv[4]);

	auto gen = [&] {
		if (argc == 6) {
			auto seed = stoul(argv[5]);
			return mt19937(seed);
		}
		random_device rd;
		return mt19937(rd());
	}();

	size_t count = 0;
	size_t connected = 0;
	size_t minimal = 0;

	while (true) {
		auto const m = generate_random_machine(N, P, Q, gen);
		auto const m2 = reachable_submachine(m, 0);
		auto const tree = create_splitting_tree(m2, min_hopcroft_style, 0).root;

		count++;
		if (m.graph_size == m2.graph_size) connected++;
		if (number_of_leaves(tree) == m.graph_size) minimal++;

		if (number_of_leaves(tree) == m.graph_size) {
			print_machine(m2, minimal);
		}

		if (minimal >= number_of_machines) break;
	}

	clog << minimal << " / " << connected << " / " << count << endl;
}
