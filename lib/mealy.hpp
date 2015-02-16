#pragma once

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
struct Mealy {
	struct edge {
		size_t to = -1;
		size_t output = -1;
	};

	std::map<std::string, size_t> nodes_indices;
	std::map<std::string, size_t> input_indices;
	std::map<std::string, size_t> output_indices;

	// state -> input -> (output, state)
	std::vector<std::vector<edge>> graph;

	// these are actually redundant!
	size_t graph_size = 0;
	size_t input_size = 0;
	size_t output_size = 0;
};

inline auto is_complete(const Mealy & m){
	for(int n = 0; n < m.graph_size; ++n){
		if(m.graph[n].size() != m.input_size) return false;
		for(auto && e : m.graph[n]) if(e.to == -1 || e.output == -1) return false;
	}
	return true;
}

inline auto apply(Mealy const & m, size_t state, size_t input){
	return m.graph[state][input];
}

template <typename Iterator>
auto apply(Mealy const & m, size_t state, Iterator b, Iterator e){
	Mealy::edge ret;
	while(b != e){
		ret = apply(m, state, *b++);
		state = ret.to;
	}
	return ret;
}
