#pragma once

#include "types.hpp"

#include <map>
#include <string>
#include <vector>

/*
 * Structure used for reading mealy files from dot files.
 * Everything is indexed by size_t's, so that we can index vectors
 * in constant time. The maps contain the original values corresponding
 * to these size_t's. Can only represent deterministic machines,
 * but partiality still can occur.
 */
struct mealy {
	struct edge {
		state to = -1;
		output output = -1;
	};

	std::map<std::string, state> nodes_indices;
	std::map<std::string, input> input_indices;
	std::map<std::string, output> output_indices;

	// state -> input -> (output, state)
	std::vector<std::vector<edge>> graph;

	// these are actually redundant!
	size_t graph_size = 0;
	size_t input_size = 0;
	size_t output_size = 0;
};

inline auto is_complete(const mealy & m){
	for(state n = 0; n < m.graph_size; ++n){
		if(m.graph[n.base()].size() != m.input_size) return false;
		for(auto && e : m.graph[n.base()]) if(e.to == -1 || e.output == -1) return false;
	}
	return true;
}

inline auto apply(mealy const & m, state state, input input){
	return m.graph[state.base()][input.base()];
}

template <typename Iterator>
auto apply(mealy const & m, state state, Iterator b, Iterator e){
	mealy::edge ret{state, -1};
	while(b != e){
		ret = apply(m, state, *b++);
		state = ret.to;
	}
	return ret;
}

// Used to invert the input_indices and output_indices maps
template <typename T>
std::vector<std::string> create_reverse_map(std::map<std::string, T> const & indices){
	std::vector<std::string> ret(indices.size());
	for(auto&& p : indices){
		ret[p.second.base()] = p.first;
	}
	return ret;
}
