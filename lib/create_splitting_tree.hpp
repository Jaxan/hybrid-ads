#pragma once

#include "mealy.hpp"
#include "splitting_tree.hpp"

#include <vector>

struct options;
struct result;


// Creates a Lee & Yannakakis style splitting tree
// Depending on the options it can also create the classical Hopcroft splitting tree
result create_splitting_tree(Mealy const & m, options opt);


// The algorithm can be altered in some ways. This struct provides options
// to the algorithm
struct options {
	bool check_validity = true;
};

constexpr options with_validity_check{true};
constexpr options without_validity_check{false};


// The algorithm constructs more than the splitting tree
// We capture the other information as well
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
