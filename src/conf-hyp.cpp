#include <mealy.hpp>
#include <read_mealy_from_dot.hpp>

#include <io.hpp>

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

template <typename T>
vector<T> resize_new(vector<T> const & in, size_t N){
	vector<T> ret(N);
	copy_n(in.begin(), N, ret.begin());
	return ret;
}

vector<string> conform(mealy const & spec, vector<string> const & spec_o_map, mealy const & impl, vector<string> const & impl_o_map, vector<vector<string>> const & suite){
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
			const auto o2 = impl_o_map[r2.output.base()];
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
	const size_t N = 136;
	vector< pair<mealy, vector<string>> > machines_and_maps(N);
	vector< vector<vector<string>>      > suites(N);
	for(int i = 0; i < N; ++i){
		cout << "reading " << i << "\r" << flush;

		const string filename = "hyp." + to_string(i) + ".obf.dot";
		machines_and_maps[i].first = read_mealy_from_dot(filename);
		machines_and_maps[i].second = create_reverse_map(machines_and_maps[i].first.output_indices);

		const string suite_filename = filename + "test_suite";
		suites[i] = open_suite(suite_filename);
	}
	cout << "done reading" << endl;

	const string red = "\x1b[31m";
	const string yellow = "\x1b[33m";
	const string cyan = "\x1B[36m";
	const string reset = "\033[0m";

	size_t fails = 0;
	for(int i = 0; i < N; ++i){
		for(int j = 0; j < N; ++j){
			cout << "checking " << i << " against " << j << "\r" << flush;
			const auto & spec = machines_and_maps[i];
			const auto & impl = machines_and_maps[j];
			const auto & suite = suites[i];
			const auto result = conform(spec.first, spec.second, impl.first, impl.second, suite);

			if(i == j && !result.empty()){
				cout << cyan << "FAIL: " << i << " " << j << " is the same machine" << reset << endl;
				fails++;
			}

			if(i < j && result.empty()){
				cout << red << "FAIL: " << i << " " << j << " no flaw detected" << reset << endl;
				fails++;
			}

			if(i > j && result.empty()){
				cout << yellow << "FAIL: " << i << " " << j << " no flaw detected" << reset << endl;
				fails++;
			}
		}
	}
	cout << "done checking" << endl;
	cout << "total of " << fails << " fails out of " << N*N << endl;
}

