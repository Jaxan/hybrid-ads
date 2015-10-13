#pragma once

#include "types.hpp"

struct mealy;

// state -> sequence going to that state
using transfer_sequences = std::vector<word>;

struct transfer_options {
	bool randomized;
};

const transfer_options randomized_transfer_sequences{true};
const transfer_options canonical_transfer_sequences{false};

transfer_sequences create_transfer_sequences(transfer_options const & opt, mealy const & machine,
                                             state s, uint_fast32_t random_seed);
