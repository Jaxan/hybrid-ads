#pragma once

#include "mealy.hpp"

#include <vector>
#include <utility>

struct dist_seq {
	dist_seq(size_t N, size_t depth)
	: CI(N)
	, depth(depth)
	{
		for(size_t i = 0; i < N; ++i)
			CI[i] = {i, i};
	}

	// current, initial
	std::vector<std::pair<state, state>> CI;
	std::vector<input> word;
	std::vector<dist_seq> children;
	size_t depth;
};

