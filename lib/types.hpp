#pragma once

#include "phantom.hpp"

#include <vector>

/* We use size_t's for easy indexing. But we do not want to mix states and
 * inputs. We use phantom typing to "generate" distinguished types :).
 */
using state = phantom<size_t, struct state_tag>;
using input = phantom<size_t, struct input_tag>;
using output = phantom<size_t, struct output_tag>;

using word = std::vector<input>;
