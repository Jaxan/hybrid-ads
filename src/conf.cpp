#include <mealy.hpp>
#include <read_mealy_from_dot.hpp>

#include <io.hpp>

#include <fstream>
#include <iostream>
#include <map>
#include <string>

using namespace std;

template <typename T>
vector<T> resize_new(vector<T> const & in, size_t N){
	vector<T> ret(N);
	copy_n(in.begin(), N, ret.begin());
	return ret;
}

struct mealy_with_maps {
	mealy machine;
	map<string, input> inputs;
	vector<string> outputs;
};

vector<string> conform(mealy_with_maps const & spec, mealy_with_maps const & impl, vector<vector<string>> const & suite){
	for(auto && test : suite){
		state s = 0;
		state t = 0;

		size_t count = 0;
		for(auto && i : test){
			const auto i1 = spec.inputs.at(i);
			const auto r1 = apply(spec.machine, s, i1);
			const auto o1 = spec.outputs[r1.output.base()];
			s = r1.to;

			const auto i2 = impl.inputs.at(i);
			const auto r2 = apply(impl.machine, t, i2);
			const auto o2 = impl.outputs[r2.output.base()];
			t = r2.to;

			if(o1 != o2){
				return resize_new(test, count+1);
			}
			count++;
		}
	}

	return {};
}

vector<vector<string>> open_suite(string suite_filename){
	boost::iostreams::filtering_istream suite_file;
	suite_file.push(boost::iostreams::gzip_decompressor());
	suite_file.push(boost::iostreams::file_descriptor_source(suite_filename));
	boost::archive::text_iarchive archive(suite_file);

	vector<vector<string>> suite;
	archive >> suite;
	return suite;
}

int main(int argc, char *argv[]){
	if(argc != 3) return 1;

	const auto spec = read_mealy_from_dot(argv[1]);
	const auto spec_o_map = create_reverse_map(spec.second.output_indices);

	const auto impl = read_mealy_from_dot(argv[2]);
	const auto impl_o_map = create_reverse_map(impl.second.output_indices);

	const auto suite = open_suite(argv[1] + string("test_suite"));

	const auto counter_example = conform({spec.first, spec.second.input_indices, spec_o_map}, {impl.first, impl.second.input_indices, impl_o_map}, suite);
	if(counter_example.empty()){
		cerr << "No counter example found" << endl;
	}
	for(auto && i : counter_example) cout << i << ' ';
	cout << endl;
}

