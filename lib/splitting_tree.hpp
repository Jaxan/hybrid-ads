#pragma once

#include "mealy.hpp"

/// \brief A splitting tree as defined in Lee & Yannakakis.
/// This is also known as a derivation tree (Knuutila). Both the Gill/Moore/Hopcroft-style and the
/// Lee&Yannakakis-style trees are splitting trees.
struct splitting_tree {
	splitting_tree(size_t N, size_t depth);

	std::vector<state> states;
	std::vector<splitting_tree> children;
	word separator;
	size_t depth = 0;
	mutable int mark = 0; // used for some algorithms...
};

template <typename Fun> void lca_impl1(splitting_tree const & node, Fun && f) {
	node.mark = 0;
	if (!node.children.empty()) {
		for (auto && c : node.children) {
			lca_impl1(c, f);
			if (c.mark) node.mark++;
		}
	} else {
		for (auto && s : node.states) {
			if (f(s)) node.mark++;
		}
	}
}

splitting_tree & lca_impl2(splitting_tree & node);

/// \brief Find the lowest common ancestor of elements on which \p f returns true.
template <typename Fun> splitting_tree & lca(splitting_tree & root, Fun && f) {
	static_assert(std::is_same<decltype(f(0)), bool>::value, "f should return a bool");
	lca_impl1(root, f);
	return lca_impl2(root);
}

template <typename Fun> const splitting_tree & lca(const splitting_tree & root, Fun && f) {
	static_assert(std::is_same<decltype(f(0)), bool>::value, "f should return a bool");
	lca_impl1(root, f);
	return lca_impl2(const_cast<splitting_tree &>(root));
}


/// \brief Structure contains options to alter the splitting tree creation.
/// \p check_validity checks whether the transition/output map is injective on the current set of
/// nodes which is being split. Setting this false degenerates to generating pairwise separating
/// sequences. \p assert_minimal_order is used to produce minimal (pairwise) separating sequences.
/// \p cach_succesors is needed by the second step in the LY algorithm and \p randomized randomizes
/// the loops over the alphabet.
struct options {
	bool check_validity;
	bool assert_minimal_order;
	bool cache_succesors;
	bool randomized;
};

const options lee_yannakakis_style = {true, false, true, false};
const options hopcroft_style = {false, false, false, false};
const options min_hopcroft_style = {false, true, false, false};
const options randomized_lee_yannakakis_style = {true, false, true, true};
const options randomized_hopcroft_style = {false, false, false, true};
const options randomized_min_hopcroft_style = {false, true, false, true};

/// \brief The algorithm produces more than just a splitting tree, all results are put here.
struct result {
	result(size_t N) : root(N, 0), successor_cache(), is_complete(N <= 1) {}

	// The splitting tree as described in Lee & Yannakakis
	splitting_tree root;

	// Encodes f_u : depth -> state -> state, where only the depth of u is of importance
	std::vector<std::vector<state>> successor_cache;

	// false <-> no adaptive distinguishing sequence
	bool is_complete;
};

/// \brief Creates a splitting tree by partition refinement.
/// \returns a splitting tree and other calculated structures.
result create_splitting_tree(mealy const & m, options opt);
