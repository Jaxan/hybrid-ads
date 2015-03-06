#include <adaptive_distinguishing_sequence.hpp>
#include <logging.hpp>
#include <mealy.hpp>
#include <read_mealy_from_dot.hpp>
#include <seperating_family.hpp>
#include <seperating_matrix.hpp>
#include <splitting_tree.hpp>
#include <transfer_sequences.hpp>

#include <io.hpp>

#include <future>

using namespace std;

template <typename T>
vector<string> create_reverse_map(map<string, T> const & indices){
	vector<string> ret(indices.size());
	for(auto&& p : indices){
		ret[p.second.base()] = p.first;
	}
	return ret;
}

template <typename T>
std::vector<T> concat(std::vector<T> const & l, std::vector<T> const & r){
	std::vector<T> ret(l.size() + r.size());
	auto it = copy(begin(l), end(l), begin(ret));
	copy(begin(r), end(r), it);
	return ret;
}

template <typename T>
std::vector<std::vector<T>> all_seqs(T min, T max, std::vector<std::vector<T>> const & seqs){
	std::vector<std::vector<T>> ret((max - min) * seqs.size());
	auto it = begin(ret);
	for(auto && x : seqs){
		for(T i = min; i < max; ++i){
			it->assign(x.size()+1);
			auto e = copy(x.begin(), x.end(), it->begin());
			*e++ = i;
		}
	}
	return ret;
}

int main(int argc, char *argv[]){
	if(argc != 2) return 1;
	const string filename = argv[1];
	const bool use_stdio = filename == "--";

	const auto machine = [&]{
		timer t("reading file " + filename);
		if(use_stdio){
			return read_mealy_from_dot(cin);
		} else {
			return read_mealy_from_dot(filename);
		}
	}();

	auto all_pair_seperating_sequences_fut = async([&]{
		const auto splitting_tree_hopcroft = [&]{
			timer t("creating hopcroft splitting tree");
			return create_splitting_tree(machine, hopcroft_style);
		}();

		const auto all_pair_seperating_sequences = [&]{
			timer t("gathering all seperating sequences");
			return create_all_pair_seperating_sequences(splitting_tree_hopcroft.root);
		}();

		return all_pair_seperating_sequences;
	});

	auto sequence_fut = async([&]{
		const auto splitting_tree = [&]{
			timer t("Lee & Yannakakis I");
			return create_splitting_tree(machine, lee_yannakakis_style);
		}();

		const auto sequence = [&]{
			timer t("Lee & Yannakakis II");
			return create_adaptive_distinguishing_sequence(splitting_tree);
		}();

		return sequence;
	});

	auto transfer_sequences_fut = std::async([&]{
		timer t("determining transfer sequences");
		return create_transfer_sequences(machine, 0);
	});

	const auto all_pair_seperating_sequences = all_pair_seperating_sequences_fut.get();
	const auto sequence = sequence_fut.get();

	const auto seperating_family = [&]{
		timer t("making seperating family");
		return create_seperating_family(sequence, all_pair_seperating_sequences);
	}();

	const auto transfer_sequences = transfer_sequences_fut.get();
	const auto inputs = create_reverse_map(machine.input_indices);

	{
		timer t("making test suite");
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
		if(use_stdio){
			compressed_stream.push(cout);
		} else {
			compressed_stream.push(boost::iostreams::file_descriptor_sink(filename + "test_suite"));
		}

		boost::archive::text_oarchive archive(compressed_stream);
		archive << real_suite;
	}
}

