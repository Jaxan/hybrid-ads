#include <io.hpp>
#include <mealy.hpp>
#include <read_mealy_from_dot.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>

using namespace std;

void print_stats_for_machine(string filename){
	const auto machine = read_mealy_from_dot(filename).first;

	cout << "machine " << filename << " has\n";
	cout << '\t' << machine.graph_size << " states\n";
	cout << '\t' << machine.input_size << " inputs\n";
	cout << '\t' << machine.output_size << " outputs\n";
}

void print_stats_for_suite(string filename){
	boost::iostreams::filtering_istream suite_file;
	suite_file.push(boost::iostreams::gzip_decompressor());
	suite_file.push(boost::iostreams::file_descriptor_source(filename));
	boost::archive::text_iarchive archive(suite_file);

	vector<vector<string>> suite;
	archive >> suite;

	const auto & longest = *max_element(suite.begin(), suite.end(), [](const auto & x, const auto & y){ return x.size() < y.size(); });
	const auto total_length = accumulate(suite.begin(), suite.end(), 0, [](const auto & x, const auto & y){ return x + y.size(); });

	cout << "suite " << filename << " has\n";
	cout << '\t' << suite.size() << " sequences\n";
	cout << '\t' << total_length << " input symbols in total\n";
	cout << '\t' << double(total_length) / suite.size() << " input symbols on average per test\n";
	cout << '\t' << longest.size() << " inputs symbols in the longest test\n";
}

int main(int argc, char *argv[]){
	if(argc != 2) return 37;

	const string filename = argv[1];
	if(filename.find("test_suite") == string::npos){
		print_stats_for_machine(filename);
	} else {
		print_stats_for_suite(filename);
	}
}

