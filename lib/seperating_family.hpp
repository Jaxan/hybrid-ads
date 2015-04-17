#pragma once

#include "adaptive_distinguishing_sequence.hpp"
#include "seperating_matrix.hpp"
#include "types.hpp"

/// \brief From the LY algorithm we generate characterizations sets (as in the Chow framework)
/// If the adaptive distinguihsing sequence is complete, then we do not need to augment the LY
/// result. This results in a separating family, which is stronger than a characterization set.
/// However, if it is not complete, we augment it with sequences from the Wp-method.

/// \brief A set (belonging to some state) of characterizing sequences
/// It contains global_suffixes which should be used for testing whether the state is correct. Once
/// we know the states make sense, we can test the transitions with the smaller set local_suffixes.
/// There is some redundancy in this struct, but we have plenty of memory at our disposal.
/// Note that even the global_suffixes may really on the state (because of the adaptiveness of the
/// LY distinguishing sequence).
struct characterization_set {
	std::vector<word> global_suffixes;
	std::vector<word> local_suffixes;
};

/// \brief A family (indexed by states) of characterizations
using characterization_family = std::vector<characterization_set>;

/// \brief Creates the characterization family from the results of the LY algorithm
/// If the sequence is complete, we do not need the separating_matrix
characterization_family create_seperating_family(const adaptive_distinguishing_sequence & sequence,
                                                 const seperating_matrix & sep_matrix);
