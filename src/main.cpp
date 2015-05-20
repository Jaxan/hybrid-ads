#include <adaptive_distinguishing_sequence.hpp>
#include <logging.hpp>
#include <mealy.hpp>
#include <reachability.hpp>
#include <read_mealy_from_dot.hpp>
#include <read_mealy_from_txt.hpp>
#include <characterization_family.hpp>
#include <separating_matrix.hpp>
#include <splitting_tree.hpp>
#include <transfer_sequences.hpp>

#include <future>
#include <iomanip>
#include <numeric>
#include <random>

using namespace std;

using time_logger = silent_timer;

int main(int argc, char *argv[]) try {
	if(argc != 4) {
		cerr << "usage: main <filename> <max k> <stream|stop>" << endl;
		return 1;
	}
	const string filename = argv[1];
	const bool use_stdio = filename == "--";

	// 0 => only states checks. 1 => transition checks. 2 or more => deep checks
	const auto k_max = stoul(argv[2]);

	const string mode = argv[3];
	const bool streaming = mode == "stream" || mode == "stop";
	const bool random_part = streaming && mode != "stop";

	const bool use_distinguishing_sequence = true;
	const bool use_relevances = false;
	const bool randomize_prefixes = true;
	const bool randomize_hopcroft = true;
	const bool randomize_lee_yannakakis = true;

	const auto machine_and_translation = [&]{
		time_logger t("reading file " + filename);
		if(use_stdio){
			return read_mealy_from_dot(cin);
		}
		if(filename.find(".txt") != string::npos) {
			const auto m = read_mealy_from_txt(filename);
			const auto t = create_translation_for_mealy(m);
			return make_pair(move(m), move(t));
		} else if (filename.find(".dot") != string::npos) {
			return read_mealy_from_dot(filename);
		}

		clog << "warning: unrecognized file format, assuming .dot\n";
		return read_mealy_from_dot(filename);
	}();

	const auto & machine = reachable_submachine(move(machine_and_translation.first), 0);
	const auto & translation = machine_and_translation.second;

	auto all_pair_seperating_sequences = [&]{
		const auto splitting_tree_hopcroft = [&]{
			time_logger t("creating hopcroft splitting tree");
			return create_splitting_tree(machine, randomize_hopcroft ? randomized_hopcroft_style : hopcroft_style);
		}();

		const auto all_pair_seperating_sequences = [&]{
			time_logger t("gathering all seperating sequences");
			return create_all_pair_seperating_sequences(splitting_tree_hopcroft.root);
		}();

		return all_pair_seperating_sequences;
	}();

	auto sequence = [&]{
		const auto tree = [&]{
			time_logger t("Lee & Yannakakis I");
			if(use_distinguishing_sequence)
				return create_splitting_tree(machine, randomize_lee_yannakakis ? randomized_lee_yannakakis_style : lee_yannakakis_style);
			else
				return result(machine.graph_size);
		}();

		const auto sequence = [&]{
			time_logger t("Lee & Yannakakis II");
			return create_adaptive_distinguishing_sequence(tree);
		}();

		return sequence;
	}();

	auto transfer_sequences = [&]{
		time_logger t("determining transfer sequences");
		if(randomize_prefixes){
			return create_randomized_transfer_sequences(machine, 0);
		} else {
			return create_transfer_sequences(machine, 0);
		}
	}();

	auto inputs = [&]{
		return create_reverse_map(translation.input_indices);
	}();

	auto relevant_inputs = [&]{
		time_logger t("determining relevance of inputs");
		vector<discrete_distribution<input>> distributions(machine.graph_size);

		for(state s = 0; s < machine.graph_size; ++s){
			vector<double> r_cache(machine.input_size, 1);
			if(use_relevances){
				for(input i = 0; i < machine.input_size; ++i){
					//const auto test1 = apply(machine, s, i).output != machine.output_indices.at("quiescence");
					const auto test2 = apply(machine, s, i).to != s;
					r_cache[i] = 0.1 + test2;
				}
			}

			// VS 2013 is missing some c++11 support: http://stackoverflow.com/questions/21959404/initialising-stddiscrete-distribution-in-vs2013
			size_t i = 0;
			distributions[s] = discrete_distribution<input>(r_cache.size(), r_cache.front(), r_cache.back(), [&r_cache, &i](double) { return r_cache[i++]; });
		}
		return distributions;
	}();

	// const auto all_pair_seperating_sequences = all_pair_seperating_sequences_fut.get();
	// const auto sequence = sequence_fut.get();

	const auto seperating_family = [&]{
		time_logger t("making seperating family");
		return create_seperating_family(sequence, all_pair_seperating_sequences);
	}();

	// const auto transfer_sequences = transfer_sequences_fut.get();
	// const auto inputs = inputs_fut.get();

	const auto print_word = [&](vector<input> w){
		for(auto && x : w) cout << inputs[x] << ' ';
	};

	if(streaming){
		time_logger t("outputting all preset tests");

		vector<word> all_sequences(1);
		for(size_t k = 0; k <= k_max; ++k){
			clog << "*** K = " << k << endl;
			for(state s = 0; s < machine.graph_size; ++s){
				const auto prefix = transfer_sequences[s];

				for(auto && suffix : seperating_family[s].local_suffixes){
					for(auto && r : all_sequences){
						print_word(prefix);
						print_word(r);
						print_word(suffix);
						cout << endl;
					}
				}
			}

			all_sequences = all_seqs(0, machine.input_size, all_sequences);
		}
	}

	if(random_part){
		time_logger t("outputting all random tests");
		clog << "*** K > " << k_max << endl;

		std::random_device rd;
		std::mt19937 generator(rd());

		uniform_int_distribution<size_t> prefix_selection(0, transfer_sequences.size()-1);
		uniform_int_distribution<> unfair_coin(0, 2); // expected flips is p / (p-1)^2, where p is succes probability
		uniform_int_distribution<size_t> suffix_selection;
		// auto relevant_inputs = relevant_inputs_fut.get();

		while(true){
			state current_state = 0;

			const auto & p = transfer_sequences[prefix_selection(generator)];
			current_state = apply(machine, current_state, begin(p), end(p)).to;

			vector<input> m;
			m.reserve(k_max + 2);
			size_t minimal_size = k_max + 1;
			while(minimal_size || unfair_coin(generator)){
				input i = relevant_inputs[current_state](generator);
				m.push_back(i);
				current_state = apply(machine, current_state, i).to;
				if(minimal_size) minimal_size--;
			}

			using params = uniform_int_distribution<size_t>::param_type;
			const auto & suffixes = seperating_family[current_state].local_suffixes;
			const auto & s = suffixes[suffix_selection(generator, params{0, suffixes.size()-1})];

			print_word(p);
			print_word(m);
			print_word(s);
			cout << endl;
		}
	}
} catch (exception const & e) {
	cerr << "Exception thrown: " << e.what() << endl;
	return 1;
}
