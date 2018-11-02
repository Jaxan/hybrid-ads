#pragma once

#include "mealy.hpp"
#include "separating_family.hpp"
#include "transfer_sequences.hpp"
#include "types.hpp"

#include <functional>
#include <vector>

struct writer {
	std::function<void(word)> apply; // store a part of a word
	std::function<bool(void)> reset; // flush, if flase is returned, testing is stopped
};

/// \brief Performs exhaustive tests with mid sequences < \p k_max (harmonized, e.g. HSI / DS)
void test(mealy const & specification, transfer_sequences const & prefixes,
          separating_family const & separating_family, size_t k_max, writer const & output);

void test(const mealy & specification, const transfer_sequences & prefixes,
          std::vector<word> & all_sequences, const separating_family & separating_family,
          size_t k_max, const writer & output);

/// \brief Performs random non-exhaustive tests for more states (harmonized, e.g. HSI / DS)
void randomized_test(mealy const & specification, transfer_sequences const & prefixes,
                     separating_family const & separating_family, size_t min_k, size_t rnd_length,
                     writer const & output, uint_fast32_t random_seed);

void randomized_test_suffix(mealy const & specification, transfer_sequences const & prefixes,
                            separating_family const & separating_family, size_t min_k,
                            size_t rnd_length, writer const & output, uint_fast32_t random_seed);

/// \brief returns a writer which simply writes everything to cout (via inputs)
writer default_writer(const std::vector<std::string> & inputs, std::ostream & os);
