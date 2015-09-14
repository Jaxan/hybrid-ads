#include <adaptive_distinguishing_sequence.hpp>
#include <logging.hpp>
#include <mealy.hpp>
#include <reachability.hpp>
#include <read_mealy.hpp>
#include <separating_family.hpp>
#include <splitting_tree.hpp>
#include <test_suite.hpp>
#include <transfer_sequences.hpp>

#include <algorithm>
#include <future>
#include <iomanip>
#include <numeric>
#include <random>

using namespace std;

using time_logger = silent_timer;

int main(int argc, char *argv[]) try {
	if(argc != 5 && argc != 6) {
		cerr << "usage: main <filename> <max k> <rnd length> <all|fixed|random> [<seed>]" << endl;
		return 1;
	}
	const string filename = argv[1];
	const bool use_stdio = filename == "--";

	const auto k_max = stoul(argv[2]);
	const auto rnd_length = stoul(argv[3]);

	const string mode = argv[4];
	const bool streaming = mode == "all" || mode == "fixed";
	const bool random_part = mode == "all" || mode == "random";

	const bool seed_provided = argc == 6;
	const uint_fast32_t seed = seed_provided ? stoul(argv[5]) : 0;

	const bool use_distinguishing_sequence = true;
	const bool randomize_prefixes = true;
	const bool randomize_hopcroft = true;
	const bool randomize_lee_yannakakis = true;

	const auto machine_and_translation = [&]{
		time_logger t_("reading file " + filename);
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

	// every thread gets its own seed
	const auto random_seeds = [&] {
		vector<uint_fast32_t> seeds(4);
		if (seed_provided) {
			seed_seq s{seed};
			s.generate(seeds.begin(), seeds.end());
		} else {
			random_device rd;
			generate(seeds.begin(), seeds.end(), ref(rd));
		}
		return seeds;
	}();

	auto all_pair_separating_sequences = [&]{
		const auto splitting_tree_hopcroft = [&]{
			time_logger t("creating hopcroft splitting tree");
			return create_splitting_tree(machine, randomize_hopcroft ? randomized_hopcroft_style : hopcroft_style, random_seeds[0]);
		}();

		return splitting_tree_hopcroft.root;
	}();

	auto sequence = [&]{
		const auto tree = [&]{
			time_logger t("Lee & Yannakakis I");
			if(use_distinguishing_sequence)
				return create_splitting_tree(machine, randomize_lee_yannakakis ? randomized_lee_yannakakis_style : lee_yannakakis_style, random_seeds[1]);
			else
				return result(machine.graph_size);
		}();

		const auto sequence_ = [&]{
			time_logger t("Lee & Yannakakis II");
			return create_adaptive_distinguishing_sequence(tree);
		}();

		return sequence_;
	}();

	auto transfer_sequences = [&]{
		time_logger t("determining transfer sequences");
		if(randomize_prefixes){
			return create_randomized_transfer_sequences(machine, 0, random_seeds[2]);
		} else {
			return create_transfer_sequences(machine, 0);
		}
	}();

	auto inputs = create_reverse_map(translation.input_indices);

	const auto separating_family = [&]{
		time_logger t("making seperating family");
		return create_separating_family(sequence, all_pair_separating_sequences);
	}();

	if(streaming){
		time_logger t("outputting all preset tests");
		test(machine, transfer_sequences, separating_family, k_max, default_writer(inputs));
	}

	if(random_part){
		time_logger t("outputting all random tests");
		const auto k_max_ = streaming ? k_max + 1 : 0;
		randomized_test(machine, transfer_sequences, separating_family, k_max_, rnd_length, default_writer(inputs), random_seeds[3]);
	}

} catch (exception const & e) {
	cerr << "Exception thrown: " << e.what() << endl;
	return 1;
}
