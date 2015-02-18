#pragma once

#include "mealy.hpp"

#include <numeric>
#include <type_traits>
#include <vector>

struct splijtboom {
	splijtboom(size_t N, size_t depth)
	: states(N)
	, depth(depth)
	{
		std::iota(begin(states), end(states), 0);
	}

	std::vector<state> states;
	std::vector<splijtboom> children;
	std::vector<input> seperator;
	size_t depth = 0;
	int mark = 0; // used for some algorithms...
};

template <typename Fun>
void lca_impl1(splijtboom & node, Fun && f){
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

inline splijtboom & lca_impl2(splijtboom & node){
	if(node.mark > 1) return node;
	for(auto && c : node.children){
		if(c.mark > 0) return lca_impl2(c);
	}
	return node; // this is a leaf
}

template <typename Fun>
splijtboom & lca(splijtboom & root, Fun && f){
	static_assert(std::is_same<decltype(f(0)), bool>::value, "f should return a bool");
	lca_impl1(root, f);
	return lca_impl2(root);
}
