#pragma once

#include "types.hpp"
#include "splitting_tree.hpp"

/*
 * A seperating matrix is a matrix indexed by states, which assigns to each
 * pair of (inequivalent) states a seperating sequences. This can be done by
 * the classical Hopcroft algorithm
 */

using seperating_row = std::vector<word>;
using seperating_matrix = std::vector<seperating_row>;

seperating_matrix create_all_pair_seperating_sequences(splitting_tree const & root);
