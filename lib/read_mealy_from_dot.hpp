#pragma once

#include "types.hpp"

#include <iosfwd>
#include <map>
#include <utility>

/*
 * These maps record the translation used while reading.
 */
struct translation {
	std::map<std::string, input> input_indices;
	input max_input = 0;

	std::map<std::string, output> output_indices;
	output max_output = 0;
};

struct mealy;

// Read a mealy machine while extending the translation
mealy read_mealy_from_dot(std::istream & in, translation & t);
mealy read_mealy_from_dot(const std::string & filename, translation & t);

// Read a mealy machine, starting with the empty translation
std::pair<mealy, translation> read_mealy_from_dot(std::istream & in);
std::pair<mealy, translation> read_mealy_from_dot(const std::string & filename);

// Used to invert the input_indices and output_indices maps
template <typename T>
std::vector<std::string> create_reverse_map(std::map<std::string, T> const & indices){
	std::vector<std::string> ret(indices.size());
	for(auto&& p : indices){
		ret[p.second.base()] = p.first;
	}
	return ret;
}
