#include <mealy.hpp>
#include <read_mealy_from_dot.hpp>

#include <io.hpp>

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

template <typename T>
vector<string> create_reverse_map(map<string, T> const & indices){
	vector<string> ret(indices.size());
	for(auto&& p : indices){
		ret[p.second.base()] = p.first;
	}
	return ret;
}

int main(int argc, char *argv[]){
	if(argc != 4) return 37;

	const string spec_filename = argv[1];
	const string impl_filename = argv[2];
	const string suite_filename = argv[3];

	ifstream spec_file(spec_filename);
	ifstream impl_file(impl_filename);

	boost::iostreams::filtering_istream suite_file;
	suite_file.push(boost::iostreams::gzip_decompressor());
	suite_file.push(boost::iostreams::file_descriptor_source(suite_filename));
	boost::archive::text_iarchive archive(suite_file);

	const auto spec = read_mealy_from_dot(spec_file);
	const auto impl = read_mealy_from_dot(impl_file);

	const auto spec_o_map = create_reverse_map(spec.output_indices);
	const auto impl_o_map = create_reverse_map(impl.output_indices);

	vector<vector<string>> suite;
	archive >> suite;

	size_t tcount = 0;
	for(auto && test : suite){
		state s = 0;
		state t = 0;

		size_t count = 0;
		for(auto && i : test){
			const auto i1 = spec.input_indices.at(i);
			const auto r1 = apply(spec, s, i1);
			const auto o1 = spec_o_map[r1.output.base()];
			s = r1.to;

			const auto i2 = spec.input_indices.at(i);
			const auto r2 = apply(impl, t, i2);
			const auto o2 = spec_o_map[r2.output.base()];
			t = r2.to;

			if(o1 != o2){
				cout << "conformance fail" << endl;
				cout << o1 << " != " << o2 << endl;
				cout << "at test " << tcount << endl;
				cout << "at char " << count << endl;
				return 1;
			}
			count++;
		}
		tcount++;
	}

	cout << "conformance succes " << tcount << endl;
}

