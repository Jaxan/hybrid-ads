#include <mealy.hpp>
#include <reachability.hpp>
#include <splitting_tree.hpp>

#include <docopt.h>

#include <iostream>
#include <random>
#include <fstream>

using namespace std;

static const char USAGE[] =
R"(Random Mealy machine generator

    Usage:
      generator random [-mc] <states> <inputs> <outputs> <machines> [<seed>]
      generator hopcroft a <states>
      generator hopcroft b <states>

    Options:
      -h, --help       Show this screen
      --version        Show version
      -m, --minimal    Only generate minimal machines
      -c, --connected  Only generate reachable machines
)";

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

static void print_machine(string const & prefix, mealy const & m, size_t count) {
	ofstream file(prefix + "_" + to_string(m.graph_size) + "_" + to_string(m.input_size) + "_"
	              + to_string(m.output_size) + "_" + to_string(count) + ".txt");
	for (state s = 0; s < m.graph_size; ++s) {
		for (input i = 0; i < m.input_size; ++i) {
			auto e = m.graph[s][i];
			file << s << " -- " << i << " / " << e.out << " -> " << e.to << endl;
		}
	}
}

int main(int argc, char * argv[]) {
	const auto args = docopt::docopt(USAGE, {argv + 1, argv + argc}, true, __DATE__ __TIME__);

	if (args.at("random").asBool()) {
		auto gen = [&] {
			if (args.at("<seed>")) {
				auto seed = args.at("<seed>").asLong();
				return mt19937(seed);
			}
			random_device rd;
			return mt19937(rd());
		}();

		size_t number_of_machines = args.at("<machines>").asLong();
		size_t constructed = 0;

		while (constructed < number_of_machines) {
			auto const m = generate_random_machine(args.at("<states>").asLong(),
			                                       args.at("<inputs>").asLong(),
			                                       args.at("<outputs>").asLong(), gen);

			if (args.at("--connected").asBool()) {
				auto const m2 = reachable_submachine(m, 0);
				if (m2.graph_size != m.graph_size) continue;
			}

			if (args.at("--minimal").asBool()) {
				auto const tree = create_splitting_tree(m, min_hopcroft_style, 0).root;
				if (number_of_leaves(tree) != m.graph_size) continue;
			}

			constructed++;
			print_machine("machine", m, constructed);
		}
	} else if (args.at("hopcroft").asBool() && args.at("a").asBool()) {
		mealy m;

		m.graph_size = args.at("<states>").asLong();
		m.input_size = m.output_size = 2;
		m.graph.assign(m.graph_size, vector<mealy::edge>(m.input_size));

		for (state s = 0; s < m.graph_size; ++s) {
			m.graph[s][0] = mealy::edge(s + 1, 0);
			m.graph[s][1] = mealy::edge(s, 0);
		}

		// "accepting state"
		m.graph[m.graph_size - 1][0] = mealy::edge(m.graph_size - 1, 1);
		m.graph[m.graph_size - 1][1] = mealy::edge(m.graph_size - 1, 1);

		print_machine("hopcroft_a", m, 1);
	} else if (args.at("hopcroft").asBool() && args.at("b").asBool()) {
		// In the original paper, the machine is not well defined...
		// So I don't know what Hopcroft had in mind exactly...
		mealy m;

		auto n = m.graph_size = args.at("<states>").asLong();
		m.input_size = m.output_size = 2;
		m.graph.assign(m.graph_size, vector<mealy::edge>(m.input_size));

		for (state s = 0; s < n; ++s) {
			m.graph[s][0] = mealy::edge(s ? s - 1 : 0, s < n / 2 ? 0 : 1);
			if (s < n / 2) {
				m.graph[s][1] = mealy::edge(2 * s + 1, 0);
			} else {
				m.graph[s][1] = mealy::edge(s + (n - s) / 2, 0);
			}
		}

		print_machine("hopcroft_b", m, 1);
	}
}
