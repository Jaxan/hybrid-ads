#pragma once

#include "adaptive_distinguishing_sequence.hpp"
#include "create_splitting_tree.hpp"

struct result2 {
	result2(size_t N)
	: sequence(N, 0)
	{}

	// The adaptive distinguishing sequence as described in Lee & Yannakakis
	// This is really a tree!
	dist_seq sequence;
};

result2 create_adaptive_distinguishing_sequence(result const & splitting_tree);
