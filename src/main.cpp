#include <adaptive_distinguishing_sequence.hpp>
#include <logging.hpp>
#include <mealy.hpp>
#include <read_mealy_from_dot.hpp>
#include <seperating_family.hpp>
#include <seperating_matrix.hpp>
#include <splitting_tree.hpp>
#include <transfer_sequences.hpp>
#include <partition.hpp>

#include <io.hpp>

#include <future>
#include <numeric>
#include <iomanip>
#include <random>

using namespace std;

using time_logger = silent_timer;

int main(int argc, char *argv[]){
	if(argc != 4) return 1;
	const string filename = argv[1];
	const bool use_stdio = filename == "--";

	// 0 => only states checks. 1 => transition checks. 2 or more => deep checks
	const auto k_max = stoul(argv[2]);

	const string mode = argv[3];
	const bool streaming = mode == "stream";
	const bool random_part = streaming;
	const bool statistics = mode == "stats";
	const bool compress_suite = mode == "compr";

	const auto machine = [&]{
		time_logger t("reading file " + filename);
		if(use_stdio){
			return read_mealy_from_dot(cin);
		} else {
			return read_mealy_from_dot(filename);
		}
	}();

	auto all_pair_seperating_sequences_fut = async([&]{
		const auto splitting_tree_hopcroft = [&]{
			time_logger t("creating hopcroft splitting tree");
			return create_splitting_tree(machine, hopcroft_style);
		}();

		const auto all_pair_seperating_sequences = [&]{
			time_logger t("gathering all seperating sequences");
			return create_all_pair_seperating_sequences(splitting_tree_hopcroft.root);
		}();

		return all_pair_seperating_sequences;
	});

	auto sequence_fut = async([&]{
		const auto splitting_tree = [&]{
			time_logger t("Lee & Yannakakis I");
			return create_splitting_tree(machine, lee_yannakakis_style);
		}();

		const auto sequence = [&]{
			time_logger t("Lee & Yannakakis II");
			return create_adaptive_distinguishing_sequence(splitting_tree);
		}();

		return sequence;
	});

	auto transfer_sequences_fut = std::async([&]{
		time_logger t("determining transfer sequences");
		return create_transfer_sequences(machine, 0);
	});

	auto inputs_fut = std::async([&]{
		return create_reverse_map(machine.input_indices);
	});

	auto relevant_inputs_fut = std::async([&]{
		time_logger t("determining relevance of inputs");
		vector<discrete_distribution<input>> distributions(machine.graph_size);

		for(state s = 0; s < machine.graph_size; ++s){
			vector<double> r_cache(machine.input_size, 0);
			for(input i = 0; i < machine.input_size; ++i){
				const auto test1 = apply(machine, s, i).output != machine.output_indices.at("quiescence");
				const auto test2 = apply(machine, s, i).to != s;
				r_cache[i.base()] = test1 + test2;
			}

			distributions[s.base()] = discrete_distribution<input>(begin(r_cache), end(r_cache));
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

	const auto print_word = [&](auto w){
		for(auto && x : w) cout << inputs[x.base()] << ' ';
	};

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

	if(streaming){
		time_logger t("outputting all preset tests");

		vector<word> all_sequences(1);
		for(int k = 0; k <= k_max; ++k){
			cerr << "*** K = " << k << endl;
			for(state s = 0; s < machine.graph_size; ++s){
				const auto prefix = transfer_sequences[s.base()];

				for(auto && suffix : seperating_family[s.base()]){
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

		std::random_device rd;
		std::mt19937 generator(rd());

		uniform_int_distribution<size_t> prefix_selection(0, transfer_sequences.size());
		uniform_int_distribution<> fair_coin(0, 1);
		uniform_int_distribution<size_t> suffix_selection;
		auto relevant_inputs = relevant_inputs_fut.get();

		using params = uniform_int_distribution<size_t>::param_type;

		while(true){
			state current_state = 0;

			const auto & p = transfer_sequences[prefix_selection(generator)];
			current_state = apply(machine, current_state, begin(p), end(p)).to;

			vector<input> m;
			m.reserve(k_max + 2);
			size_t minimal_size = k_max + 1;
			while(minimal_size || fair_coin(generator)){
				input i = relevant_inputs[current_state.base()](generator);
				m.push_back(i);
				current_state = apply(machine, current_state, i).to;
				if(minimal_size) minimal_size--;
			}

			const auto & suffixes = seperating_family[current_state.base()];
			const auto & s = suffixes[suffix_selection(generator, params{0, suffixes.size()-1})];

			print_word(p);
			print_word(m);
			print_word(s);
			cout << endl;
		}
	}

	if(compress_suite){
		time_logger t("making test suite");
		vector<word> suite;

		for(state s = 0; s < machine.graph_size; ++s){
			const auto prefix = transfer_sequences[s.base()];

			for(auto && suffix : seperating_family[s.base()]){
				suite.push_back(concat(prefix, suffix));
			}
		}

		vector<vector<string>> real_suite(suite.size());
		transform(suite.begin(), suite.end(), real_suite.begin(), [&inputs](auto const & seq){
			vector<string> seq2(seq.size());
			transform(seq.begin(), seq.end(), seq2.begin(), [&inputs](auto const & i){
				return inputs[i.base()];
			});
			return seq2;
		});

		boost::iostreams::filtering_ostream compressed_stream;
		compressed_stream.push(boost::iostreams::gzip_compressor());
		compressed_stream.push(boost::iostreams::file_descriptor_sink(filename + "test_suite"));

		boost::archive::text_oarchive archive(compressed_stream);
		archive << real_suite;
	}
}

