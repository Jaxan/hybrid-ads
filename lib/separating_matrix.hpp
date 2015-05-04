#pragma once

#include "types.hpp"
#include "splitting_tree.hpp"


/// A separating matrix is a matrix indexed by states, which assigns to each
/// pair of (inequivalent) states a separating sequences. This can be done by
/// the classical Hopcroft or Moore algorithm
using separating_row = std::vector<word>;
using separating_matrix = std::vector<separating_row>;

separating_matrix create_all_pair_seperating_sequences(splitting_tree const & root);
