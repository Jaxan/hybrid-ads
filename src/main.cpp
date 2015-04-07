#include <adaptive_distinguishing_sequence.hpp>
#include <logging.hpp>
#include <mealy.hpp>
#include <read_mealy_from_dot.hpp>
#include <seperating_family.hpp>
#include <seperating_matrix.hpp>
#include <splitting_tree.hpp>
#include <transfer_sequences.hpp>

#include <future>
#include <iomanip>
#include <numeric>
#include <random>

using namespace std;

using time_logger = silent_timer;

int main(int argc, char *argv[]) try {
	if(argc != 4) return 1;
	const string filename = argv[1];
	const bool use_stdio = filename == "--";

	// 0 => only states checks. 1 => transition checks. 2 or more => deep checks
	const auto k_max = stoul(argv[2]);

	const string mode = argv[3];
	const bool streaming = mode == "stream";
	const bool random_part = streaming;
	const bool statistics = mode == "stats";

	const bool use_distinguishing_sequence = true;
	const bool use_relevances = false;
	const bool randomize_prefixes = true;
	const bool randomize_hopcroft = true;
	const bool randomize_lee_yannakakis = true;

	const auto machine_and_translation = [&]{
		time_logger t("reading file " + filename);
		if(use_stdio){
			return read_mealy_from_dot(cin);
		} else {
			return read_mealy_from_dot(filename);
		}
	}();

	const auto & machine = machine_and_translation.first;
	const auto & translation = machine_and_translation.second;

	auto all_pair_seperating_sequences_fut = async([&]{
		const auto splitting_tree_hopcroft = [&]{
			time_logger t("creating hopcroft splitting tree");
			return create_splitting_tree(machine, randomize_hopcroft ? randomized_hopcroft_style : hopcroft_style);
		}();

		const auto all_pair_seperating_sequences = [&]{
			time_logger t("gathering all seperating sequences");
			return create_all_pair_seperating_sequences(splitting_tree_hopcroft.root);
		}();

		return all_pair_seperating_sequences;
	});

	auto sequence_fut = async([&]{
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
	});

	auto transfer_sequences_fut = std::async([&]{
		time_logger t("determining transfer sequences");
		if(randomize_prefixes){
			return create_randomized_transfer_sequences(machine, 0);
		} else {
			return create_transfer_sequences(machine, 0);
		}
	});

	auto inputs_fut = std::async([&]{
		return create_reverse_map(translation.input_indices);
	});

	auto relevant_inputs_fut = std::async([&]{
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

			distributions[s] = discrete_distribution<input>(begin(r_cache), end(r_cache));
		}
		return distributions;
	});

	const auto all_pair_seperating_sequences = all_pair_seperating_sequences_fut.get();
	const auto sequence = sequence_fut.get();

	const auto seperating_family = [&]{
		time_logger t("making seperating family");
		return create_seperating_family(sequence, all_pair_seperating_sequences);
	}();

	const auto transfer_sequences = transfer_sequences_fut.get();
	const auto inputs = inputs_fut.get();

	const auto print_word = [&](vector<input> w){
		for(auto && x : w) cout << inputs[x] << ' ';
	};

// This part is commented out, as the polymorphic lambdas are kinda important
#if 0
	if(statistics){
		const auto adder = [](auto const & x){
			return [&x](auto const & l, auto const & r) { return l + x(r); };
		};

		const auto size = adder([](auto const & r) { return r.size(); });

		const auto p_size = transfer_sequences.size();
		const auto p_total = accumulate(begin(transfer_sequences), end(transfer_sequences), 0, size);
		const auto p_avg = p_total / double(p_size);

		cout << "Prefixes:\n";
		cout << "\tsize\t" << p_size << '\n';
		cout << "\ttotal\t" << p_total << '\n';
		cout << "\tavg\t" << p_avg << '\n';

		const auto w_fam_size = seperating_family.size();
		const auto w_fam_total = accumulate(begin(seperating_family), end(seperating_family), 0, size);
		const auto w_fam_avg = w_fam_total / double(w_fam_size);

		const auto w_total = accumulate(begin(seperating_family), end(seperating_family), 0, adder([&size](auto const & r){
			return accumulate(begin(r), end(r), 0, size);
		}));
		const auto w_avg = w_total / double(w_fam_total);

		cout << "Suffixes:\n";
		cout << "\tsize\t" << w_fam_total << '\n';
		cout << "\tavg\t" << w_fam_avg << '\n';
		cout << "\ttotal\t" << w_total << '\n';
		cout << "\tavg\t" << w_avg << '\n';

		cout << "Total tests (approximately):\n";
		double total = machine.graph_size * 1 * w_fam_avg;
		double length = p_avg + 0 + w_avg;
		for(size_t k = 0; k <= k_max; ++k){
			cout << "\tk = " << k << "\t"
				 << setw(16) << size_t(total)  << " * "
				 << setw(3)  << size_t(length) << " = "
				 << setw(20) << size_t(total * length) << endl;
			total *= machine.input_size;
			length += 1;
		}
	}
#endif

	if(streaming){
		time_logger t("outputting all preset tests");

		vector<word> all_sequences(1);
		for(int k = 0; k <= k_max; ++k){
			cerr << "*** K = " << k << endl;
			for(state s = 0; s < machine.graph_size; ++s){
				const auto prefix = transfer_sequences[s];

				for(auto && suffix : seperating_family[s]){
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
		cerr << "*** K > " << k_max << endl;

		std::random_device rd;
		std::mt19937 generator(rd());

		uniform_int_distribution<size_t> prefix_selection(0, transfer_sequences.size()-1);
		uniform_int_distribution<> unfair_coin(0, 2); // expected flips is p / (p-1)^2, where p is succes probability
		uniform_int_distribution<size_t> suffix_selection;
		auto relevant_inputs = relevant_inputs_fut.get();

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
			const auto & suffixes = seperating_family[current_state];
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
