#include <adaptive_distinguishing_sequence.hpp>
#include <read_mealy_from_dot.hpp>
#include <characterization_family.hpp>
#include <separating_matrix.hpp>
#include <trie.hpp>
#include <splitting_tree.hpp>
#include <transfer_sequences.hpp>

#include <future>
#include <string>
#include <iostream>

using namespace std;

int main(int argc, char * argv[]) {
	if (argc != 4) return 1;

	const string filename = argv[1];
	const string mode = argv[2];
	const bool use_no_LY = mode == "--W-method";
	const size_t k_max = std::stoul(argv[3]);

	const auto machine = read_mealy_from_dot(filename).first;

	auto sequence_fut = async([&] {
		if (use_no_LY) {
			return create_adaptive_distinguishing_sequence(result(machine.graph_size));
		}
		const auto tree = create_splitting_tree(machine, randomized_lee_yannakakis_style);
		return create_adaptive_distinguishing_sequence(tree);
	});

	auto pairs_fut = async([&] {
		const auto tree = create_splitting_tree(machine, randomized_min_hopcroft_style);
		return create_all_pair_seperating_sequences(tree.root);
	});

	auto prefixes_fut = async([&] {
		return create_transfer_sequences(machine, 0);
	});

	auto middles_fut = async([&] {
		vector<word> all_sequences(1);
		for (size_t k = 0; k < k_max; ++k) {
			const auto new_sequences = all_seqs(0, machine.input_size, all_sequences);
			all_sequences.reserve(all_sequences.size() + new_sequences.size());
			copy(begin(new_sequences), end(new_sequences), back_inserter(all_sequences));
		}
		return all_sequences;
	});

	clog << "getting sequence and pairs" << endl;
	auto suffixes_fut = async([&] {
		return create_seperating_family(sequence_fut.get(), pairs_fut.get());
	});

	clog << "getting prefixes, middles and suffixes" << endl;
	const auto prefixes = prefixes_fut.get();
	const auto middles = middles_fut.get();
	const auto suffixes = suffixes_fut.get();
	trie test_suite;

	clog << "start testing" << endl;
	const state start = 0;
	const word empty = {};
	for (auto && p : prefixes) {
		const state s1 = apply(machine, start, begin(p), end(p)).to;
		const word w1 = concat(empty, p);
		for (auto && m : middles) {
			const state s2 = apply(machine, s1, begin(m), end(m)).to;
			const word w2 = concat(w1, m);
			const auto & suffixes_for_state = (m.size() == k_max) ? suffixes[s2].local_suffixes
			                                                      : suffixes[s2].global_suffixes;
			for (auto && s : suffixes_for_state) {
				word test = concat(w2, s);
				test_suite.insert(test);
			}
		}
	}

	const auto p = total_size(test_suite);
	cout << p.first << '\t' << p.second << endl;
}
