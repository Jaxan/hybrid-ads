#pragma once

#include "mealy.hpp"
#include "partition.hpp"
#include "splitting_tree.hpp"

#include <vector>

struct result {
	result(size_t N)
	: root(N, 0)
	, successor_cache()
	, is_complete(true)
	{}

	// The splitting tree as described in Lee & Yannakakis
	splijtboom root;

	// Encodes f_u : depth -> state -> state, where only the depth of u is of importance
	std::vector<std::vector<state>> successor_cache;

	// false <-> no adaptive distinguishing sequence
	bool is_complete;
};

result create_splitting_tree(Mealy const & m);
