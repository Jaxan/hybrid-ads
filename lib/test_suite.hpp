#pragma once

#include "characterization_family.hpp"
#include "mealy.hpp"
#include "transfer_sequences.hpp"
#include "types.hpp"

#include <functional>

struct writer {
	std::function<void(word)> apply; // store a part of a word
	std::function<void(void)> reset; // flush
};

/// \brief Performs exhaustive tests with \p k_max extra states (harmonized, e.g. HSI / DS)
void test(mealy const & specification, transfer_sequences const & prefixes,
          characterization_family const & separating_family, size_t k_max, writer const & output);

/// \brief Performs random non-exhaustive tests for more states (harmonized, e.g. HSI / DS)
[[noreturn]] void randomized_test(mealy const & specification, transfer_sequences const & prefixes,
                                  characterization_family const & separating_family, size_t min_k,
                                  writer const & output);

/// \brief returns a writer which simply writes everything to cout (via inputs)
writer default_writer(const std::vector<std::string> & inputs);
