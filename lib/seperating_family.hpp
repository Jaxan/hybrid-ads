#pragma once

#include "adaptive_distinguishing_sequence.hpp"
#include "seperating_matrix.hpp"
#include "types.hpp"

/*
 * Given an (incomplete) adaptive distinguishing sequence and all pair
 * seperating sequences, we can construct a seperating family (as defined
 * in Lee & Yannakakis). If the adaptive distinguishing sequence is complete,
 * then the all pair seperating sequences are not needed.
 */

using seperating_set = std::vector<word>;
using seperating_family = std::vector<seperating_set>;

seperating_family create_seperating_family(adaptive_distinguishing_sequence const & sequence, seperating_matrix const & all_pair_seperating_sequences);
