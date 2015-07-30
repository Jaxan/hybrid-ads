#include <adaptive_distinguishing_sequence.hpp>
#include <read_mealy.hpp>
#include <separating_family.hpp>
#include <splitting_tree.hpp>
#include <transfer_sequences.hpp>
#include <trie.hpp>

#include <future>
#include <iostream>
#include <random>
#include <string>

using namespace std;

enum Method {
	hsi_method,
	ds_plus_method,
};

static Method parse_method(string const & s) {
	if (s == "--W-method") return hsi_method;
	if (s == "--DS-method") return ds_plus_method;
	throw runtime_error("No valid method specified");
}

int main(int argc, char * argv[]) {
	if (argc != 4 && argc != 5) {
		cerr << "usage: methods <file> <mode> <k_max> [<seed>]\n";
		return 1;
	}

	const string filename = argv[1];
	const Method method = parse_method(argv[2]);
	const size_t k_max = std::stoul(argv[3]);
	const bool seed_provided = argc == 5;
	const uint_fast32_t seed = seed_provided ? stoul(argv[4]) : 0;

	const auto machine = [&] {
		if (filename.find(".txt") != string::npos) {
			return read_mealy_from_txt(filename);
		} else if (filename.find(".dot") != string::npos) {
			return read_mealy_from_dot(filename).first;
		}

		clog << "warning: unrecognized file format, assuming dot";
		return read_mealy_from_dot(filename).first;
	}();

	const auto random_seeds = [&] {
		vector<uint_fast32_t> seeds(3);
		if (seed_provided) {
			seed_seq s{seed};
			s.generate(seeds.begin(), seeds.end());
		} else {
			random_device rd;
			generate(seeds.begin(), seeds.end(), ref(rd));
		}
		return seeds;
	}();

	auto sequence_fut = async([&] {
		if (method == hsi_method) {
			return create_adaptive_distinguishing_sequence(result(machine.graph_size));
		}
		const auto tree = create_splitting_tree(machine, randomized_lee_yannakakis_style, random_seeds[0]);
		return create_adaptive_distinguishing_sequence(tree);
	});

	auto pairs_fut = async([&] {
		const auto tree = create_splitting_tree(machine, randomized_min_hopcroft_style, random_seeds[1]);
		return tree.root;
	});

	auto prefixes_fut = async([&] {
		return create_randomized_transfer_sequences(machine, 0, random_seeds[2]);
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

	// clog << "getting sequence and pairs" << endl;
	auto suffixes_fut = async([&] {
		return create_separating_family(sequence_fut.get(), pairs_fut.get());
	});

	// clog << "getting prefixes, middles and suffixes" << endl;
	const auto prefixes = prefixes_fut.get();
	const auto middles = middles_fut.get();
	const auto suffixes = suffixes_fut.get();
	trie<input> test_suite;

	// clog << "start testing" << endl;
	const state start = 0;
	const word empty = {};
	for (auto && p : prefixes) {
		const state s1 = apply(machine, start, begin(p), end(p)).to;
		const word w1 = concat(empty, p);
		for (auto && m : middles) {
			const state s2 = apply(machine, s1, begin(m), end(m)).to;
			const word w2 = concat(w1, m);
			const auto & suffixes_for_state = suffixes[s2].local_suffixes;
			for (auto && s : suffixes_for_state) {
				word test = concat(w2, s);
				test_suite.insert(test);
			}
		}
	}

	const auto p = total_size(test_suite);
	cout << p.first << '\t' << p.second << '\t' << p.first + p.second << endl;
}
