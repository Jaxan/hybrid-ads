#pragma once

#include "mealy.hpp"

#include <vector>
#include <utility>

struct result;
struct distinguishing_sequence;


// Creates a distinguishing sequence based on the output of the first algorithm
distinguishing_sequence create_adaptive_distinguishing_sequence(result const & splitting_tree);


// The adaptive distinguishing sequence as described in Lee & Yannakakis
// This is really a tree!
struct distinguishing_sequence {
	distinguishing_sequence(size_t N, size_t depth)
	: CI(N)
	, depth(depth)
	{
		for(size_t i = 0; i < N; ++i)
			CI[i] = {i, i};
	}

	// current, initial
	std::vector<std::pair<state, state>> CI;
	std::vector<input> word;
	std::vector<distinguishing_sequence> children;
	size_t depth;
};
