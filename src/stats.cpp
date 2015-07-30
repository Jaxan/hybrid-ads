#include <mealy.hpp>
#include <read_mealy.hpp>
#include <reachability.hpp>
#include <transfer_sequences.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>

using namespace std;

template <typename C, typename S>
void print_quantiles(C const & container, S && selector, ostream & out) {
	const auto index_weight = [&](double p) -> pair<size_t, double> {
		auto index = (p * (container.size() - 1));
		return {floor(index), 1 - fmod(index, 1)};
	};

	auto sorted_container = container;
	sort(sorted_container.begin(), sorted_container.end(),
	     [&](auto const & l, auto const & r) { return selector(l) < selector(r); });
	out << "min/Q1/Q2/Q3/max ";
	out << selector(sorted_container.front()) << '/';

	const auto i25 = index_weight(0.25);
	out << i25.second * selector(sorted_container[i25.first])
	           + (1 - i25.second) * selector(sorted_container[i25.first + 1])
	    << '/';

	const auto i50 = index_weight(0.50);
	out << i50.second * selector(sorted_container[i50.first])
	           + (1 - i50.second) * selector(sorted_container[i50.first + 1])
	    << '/';

	const auto i75 = index_weight(0.75);
	out << i75.second * selector(sorted_container[i75.first])
	           + (1 - i75.second) * selector(sorted_container[i75.first + 1])
	    << '/';

	out << selector(sorted_container.back());
}

static auto count_self_loops(mealy const & m) {
	vector<long> ret(m.graph_size);
	for(state s = 0; s < m.graph_size; ++s){
		ret[s] = count_if(m.graph[s].begin(), m.graph[s].end(), [=](auto e){ return e.to == s; });
	}
	return ret;
}

static void print_stats_for_machine(string filename) {
	const auto machine = [&] {
		if (filename.find(".txt") != string::npos) {
			return read_mealy_from_txt(filename);
		} else if (filename.find(".dot") != string::npos) {
			return read_mealy_from_dot(filename).first;
		}

		clog << "warning: unrecognized file format, assuming dot";
		return read_mealy_from_dot(filename).first;
	}();

	cout << "machine " << filename << " has\n";
	cout << '\t' << machine.graph_size << " states\n";
	cout << '\t' << machine.input_size << " inputs\n";
	cout << '\t' << machine.output_size << " outputs" << endl;

	const auto reachable_machine = reachable_submachine(machine, 0);
	cout << '\t' << reachable_machine.graph_size << " reachable states" << endl;

	auto prefixes = create_transfer_sequences(reachable_machine, 0);
	cout << "prefixes ";
	print_quantiles(prefixes, [](auto const & l){ return l.size(); }, cout);
	cout << endl;

	auto self_loop_counts = count_self_loops(reachable_machine);
	cout << "self loops ";
	print_quantiles(self_loop_counts, [](auto const & l){ return l; }, cout);
	cout << endl;
}

int main(int argc, char * argv[]) {
	if (argc != 2) {
		cerr << "usages: stats <filename>" << endl;
		return 1;
	}

	const string filename = argv[1];
	print_stats_for_machine(filename);
}
