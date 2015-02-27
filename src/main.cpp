#include <create_adaptive_distinguishing_sequence.hpp>
#include <create_splitting_tree.hpp>
#include <logging.hpp>
#include <read_mealy_from_dot.hpp>
#include <write_tree_to_dot.hpp>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <stack>
#include <utility>
#include <vector>

using namespace std;

template <typename T>
vector<string> create_reverse_map(map<string, T> const & indices){
	vector<string> ret(indices.size());
	for(auto&& p : indices){
		ret[p.second.base()] = p.first;
	}
	return ret;
}

auto bfs(Mealy const & machine, state s){
	vector<bool> visited(machine.graph_size, false);
	vector<vector<input>> words(machine.graph_size);

	queue<state> work;
	work.push(s);
	while(!work.empty()){
		const auto u = work.front();
		work.pop();

		if(visited[u.base()]) continue;

		visited[u.base()] = true;

		for(input i = 0; i < machine.input_size; ++i){
			const auto v = apply(machine, u, i).to;
			if(visited[v.base()]) continue;

			words[v.base()] = words[u.base()];
			words[v.base()].push_back(i);
			work.push(v);
		}
	}

	return words;
}

int main(int argc, char *argv[]){
	if(argc != 2) return 1;
	const string filename = argv[1];

	const auto machine = [&]{
		timer t("reading file " + filename);
		return read_mealy_from_dot(filename);
	}();

	const auto splitting_tree_hopcroft = [&]{
		timer t("creating hopcroft splitting tree");
		return create_splitting_tree(machine, without_validity_check);
	}();

	const auto all_pair_seperating_sequences = [&]{
		timer t("gathering all seperating sequences");

		vector<vector<vector<input>>> all_pair_seperating_sequences(machine.graph_size, vector<vector<input>>(machine.graph_size));

		queue<reference_wrapper<const splijtboom>> work;
		work.push(splitting_tree_hopcroft.root);

		// total complexity is O(n^2), as we're visiting each pair only once :)
		while(!work.empty()){
			const splijtboom & node = work.front();
			work.pop();

			auto it = begin(node.children);
			auto ed = end(node.children);

			while(it != ed){
				auto jt = next(it);
				while(jt != ed){
					for(auto && s : it->states){
						for(auto && t : jt->states){
							assert(all_pair_seperating_sequences[t.base()][s.base()].empty());
							assert(all_pair_seperating_sequences[s.base()][t.base()].empty());
							all_pair_seperating_sequences[t.base()][s.base()] = node.seperator;
							all_pair_seperating_sequences[s.base()][t.base()] = node.seperator;
						}
					}
					jt++;
				}
				it++;
			}

			for(auto && c : node.children){
				work.push(c);
			}
		}

		for(size_t i = 0; i < machine.graph_size; ++i){
			for(size_t j = 0; j < machine.graph_size; ++j){
				if(i == j) continue;
				assert(!all_pair_seperating_sequences[i][j].empty());
			}
		}

		return all_pair_seperating_sequences;
	}();

	const auto splitting_tree = [&]{
		timer t("Lee & Yannakakis I");
		return create_splitting_tree(machine, with_validity_check);
	}();

	if(false){
		timer t("writing splitting tree");
		const string tree_filename = splitting_tree.is_complete ? (filename + ".splitting_tree") : (filename + ".incomplete_splitting_tree");
		write_splitting_tree_to_dot(splitting_tree.root, tree_filename);
	}

	const auto sequence = [&]{
		timer t("Lee & Yannakakis II");
		return create_adaptive_distinguishing_sequence(splitting_tree);
	}();

	if(false){
		timer t("writing dist sequence");
		const string dseq_filename = splitting_tree.is_complete ? (filename + ".dist_seq") : (filename + ".incomplete_dist_seq");
		write_adaptive_distinguishing_sequence_to_dot(sequence, dseq_filename);
	}

	const auto seperating_family = [&]{
		timer t("making seperating family");
		using Word = vector<input>;
		using SepSet = vector<Word>;
		vector<SepSet> seperating_family(machine.graph_size);

		stack<pair<vector<input>, reference_wrapper<const distinguishing_sequence>>> work;
		work.push({{}, sequence});

		while(!work.empty()){
			auto word = work.top().first;
			const distinguishing_sequence & node = work.top().second;
			work.pop();

			if(node.children.empty()){
				// add sequence to this leave
				for(auto && p : node.CI){
					const auto state = p.second;
					seperating_family[state.base()].push_back(word);
				}

				// if the leaf is not a singleton, we need the all_pair seperating seqs
				for(auto && p : node.CI){
					for(auto && q : node.CI){
						const auto s = p.second;
						const auto t = q.second;
						if(s == t) continue;
						seperating_family[s.base()].push_back(all_pair_seperating_sequences[s.base()][t.base()]);
					}
				}

				continue;
			}

			for(auto && i : node.word)
				word.push_back(i);

			for(auto && c : node.children)
				work.push({word, c});
		}

		return seperating_family;
	}();

	const auto inputs = create_reverse_map(machine.input_indices);
	const auto outputs = create_reverse_map(machine.output_indices);
	const auto print_uio = [&](auto const & word, auto & out, state s) -> auto & {
		for(auto && i : word){
			const auto o = apply(machine, s, i);
			s = o.to;

			out << inputs[i.base()] << ' ' << outputs[o.output.base()] << '\n';
		}
		return out;
	};

	const auto transfer_sequences = [&]{
		timer t("determining transfer sequences");
		vector<vector<vector<input>>> transfer_sequences(machine.graph_size);
		for(state s = 0; s < machine.graph_size; ++s){
			transfer_sequences[s.base()] = bfs(machine, s);
		}
		return transfer_sequences;
	}();

	const auto short_checking_seq = [&]{
		timer t("making short checking seq");
		vector<input> big_seq;
		state from = 0;
		for(state s = from; s < machine.graph_size; ++s){
			for(const auto & seq : seperating_family[s.base()]){
				copy(begin(seq), end(seq), back_inserter(big_seq));
				from = apply(machine, s, begin(seq), end(seq)).to;

				const auto to = s;
				if(from == to) continue;

				const auto transfer = transfer_sequences[from.base()][to.base()];
				copy(begin(transfer), end(transfer), back_inserter(big_seq));
			}

			const auto to = s+1;
			if(from == to) continue;

			const auto transfer = transfer_sequences[from.base()][to.base()];
			copy(begin(transfer), end(transfer), back_inserter(big_seq));
		}

		return big_seq;
	}();

	{
		timer t("writing short checking seq");
		const string uios_filename = filename + ".short_check_seq";

		boost::iostreams::filtering_ostream out;
		out.push(boost::iostreams::gzip_compressor());
		out.push(boost::iostreams::file_descriptor_sink(uios_filename));

		print_uio(short_checking_seq, out, 0);
	}

	const auto long_checking_seq = [&]{
		timer t("making long checking seq");
		vector<input> big_seq;
		state from = 0;
		for(state s = from; s < machine.graph_size; ++s){
			for(input i = 0; i < machine.input_size; ++i){
				const auto t = apply(machine, s, i).to;

				for(auto && seq : seperating_family[t.base()]){
					if(from != s){
						const auto transfer = transfer_sequences[from.base()][s.base()];
						copy(begin(transfer), end(transfer), back_inserter(big_seq));
						from = s;
					}

					big_seq.push_back(i);
					from = t;

					copy(begin(seq), end(seq), back_inserter(big_seq));
					from = apply(machine, from, begin(seq), end(seq)).to;
				}
			}
		}

		return big_seq;
	}();

	{
		timer t("writing long checking seq");
		const string uios_filename = filename + ".full_check_seq";

		boost::iostreams::filtering_ostream out;
		out.push(boost::iostreams::gzip_compressor());
		out.push(boost::iostreams::file_descriptor_sink(uios_filename));

		print_uio(long_checking_seq, out, 0);
	}
}

