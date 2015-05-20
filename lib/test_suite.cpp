#include "test_suite.hpp"

#include <iostream>
#include <random>


using namespace std;

void test(const mealy & specification, const transfer_sequences & prefixes,
          const characterization_family & separating_family, size_t k_max, const writer & output) {
	vector<word> all_sequences(1);

	for (size_t k = 0; k <= k_max; ++k) {
		clog << "*** K = " << k << endl;

		for (state s = 0; s < specification.graph_size; ++s) {
			const auto prefix = prefixes[s];

			for (auto && middle : all_sequences) {
				const auto t = apply(specification, s, middle.begin(), middle.end()).to;

				for (auto && suffix : separating_family[t].local_suffixes) {
					output.apply(prefix);
					output.apply(middle);
					output.apply(suffix);
					output.reset();
				}
			}
		}

		all_sequences = all_seqs(0, specification.input_size, all_sequences);
	}
}

void randomized_test(const mealy & specification, const transfer_sequences & prefixes,
                     const characterization_family & separating_family, size_t min_k,
                     const writer & output) {
	clog << "*** K >= " << min_k << endl;

	std::random_device rd;
	std::mt19937 generator(rd());

	uniform_int_distribution<> unfair_coin(0, 2);
	uniform_int_distribution<size_t> prefix_selection(0, prefixes.size() - 1);
	uniform_int_distribution<size_t> suffix_selection;
	uniform_int_distribution<input> input_selection(0, specification.input_size - 1);

	while (true) {
		state current_state = 0;

		const auto & prefix = prefixes[prefix_selection(generator)];
		current_state = apply(specification, current_state, begin(prefix), end(prefix)).to;

		vector<input> middle;
		middle.reserve(min_k + 1);
		size_t minimal_size = min_k;
		while (minimal_size || unfair_coin(generator)) {
			input i = input_selection(generator);
			middle.push_back(i);
			current_state = apply(specification, current_state, i).to;
			if (minimal_size) minimal_size--;
		}

		using params = decltype(suffix_selection)::param_type;
		const auto & suffixes = separating_family[current_state].local_suffixes;
		const auto & suffix = suffixes[suffix_selection(generator, params{0, suffixes.size() - 1})];

		output.apply(prefix);
		output.apply(middle);
		output.apply(suffix);
		output.reset();
	}
}

writer default_writer(std::vector<std::string> const & inputs) {
	static const auto print_word = [&](word w) { for (auto && x : w) cout << inputs[x] << ' '; };
	static const auto reset = [&] { cout << endl; };
	return {print_word, reset};
}
