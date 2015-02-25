#include <mealy.hpp>
#include <read_mealy_from_dot.hpp>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[]){
	if(argc != 3) return 37;

	const string m_filename = argv[1];
	const string c_filename = argv[2];

	ifstream m_file(m_filename);
	boost::iostreams::filtering_istream c_file;
	c_file.push(boost::iostreams::gzip_decompressor());
	c_file.push(boost::iostreams::file_descriptor_source(c_filename));

	const auto machine = read_mealy_from_dot(m_file);

	string in;
	string out;

	state s = 0;
	size_t count = 0;
	while(c_file >> in >> out){
		const auto i = machine.input_indices.at(in);
		const auto o = machine.output_indices.at(out);

		const auto ret = apply(machine, s, i);
		if(ret.output != o){
			cout << "conformance fail" << endl;
			cout << ret.output << " != " << o << endl;
			cout << "at index " << count << endl;
			return 1;
		}

		s = ret.to;
		count++;
	}

	cout << "conformance succes " << count << endl;
}

