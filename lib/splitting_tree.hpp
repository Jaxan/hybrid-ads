#pragma once

#include "mealy.hpp"

/*
 * A splitting tree as defined in Lee & Yannakakis. The structure is also
 * called a derivation tree in Knuutila. Both the classical Hopcroft algorithm
 * and the Lee & Yannakakis algorithm produce splitting trees.
 */

struct splitting_tree {
	splitting_tree(size_t N, size_t depth);

	std::vector<state> states;
	std::vector<splitting_tree> children;
	word seperator;
	size_t depth = 0;
	mutable int mark = 0; // used for some algorithms...
};

template <typename Fun>
void lca_impl1(splitting_tree const & node, Fun && f){
	node.mark = 0;
	if(!node.children.empty()){
		for(auto && c : node.children){
			lca_impl1(c, f);
			if(c.mark) node.mark++;
		}
	} else {
		for(auto && s : node.states){
			if(f(s)) node.mark++;
		}
	}
}

splitting_tree & lca_impl2(splitting_tree & node);

template <typename Fun>
splitting_tree & lca(splitting_tree & root, Fun && f){
	static_assert(std::is_same<decltype(f(0)), bool>::value, "f should return a bool");
	lca_impl1(root, f);
	return lca_impl2(root);
}

template <typename Fun>
const splitting_tree & lca(const splitting_tree & root, Fun && f){
	static_assert(std::is_same<decltype(f(0)), bool>::value, "f should return a bool");
	lca_impl1(root, f);
	return lca_impl2(const_cast<splitting_tree&>(root));
}


/*
 * The algorithm to create a splitting tree can be altered in some ways. This
 * struct provides options to the algorithm. There are two common setups.
 */

struct options {
	bool check_validity = true;
	bool cache_succesors = true;
	bool randomized = false;
};

constexpr options lee_yannakakis_style{true, true, false};
constexpr options hopcroft_style{false, false, false};
constexpr options randomized_lee_yannakakis_style{true, true, true};
constexpr options randomized_hopcroft_style{false, false, true};

/*
 * The algorithm to create a splitting tree also produces some other useful
 * data. This struct captures exactly that.
 */

struct result {
	result(size_t N)
	: root(N, 0)
	, successor_cache()
	, is_complete(true)
	{}

	// The splitting tree as described in Lee & Yannakakis
	splitting_tree root;

	// Encodes f_u : depth -> state -> state, where only the depth of u is of importance
	std::vector<std::vector<state>> successor_cache;

	// false <-> no adaptive distinguishing sequence
	bool is_complete;
};

result create_splitting_tree(mealy const & m, options opt);
