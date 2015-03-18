#pragma once

#include "types.hpp"

struct mealy;

using transfer_sequences = std::vector<word>;

transfer_sequences create_transfer_sequences(mealy const & machine, state s);
transfer_sequences create_randomized_transfer_sequences(mealy const & machine, state s);
