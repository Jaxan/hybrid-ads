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

// concattenation of words
template <typename T>
std::vector<T> concat(std::vector<T> const & l, std::vector<T> const & r){
	std::vector<T> ret(l.size() + r.size());
	auto it = copy(begin(l), end(l), begin(ret));
	copy(begin(r), end(r), it);
	return ret;
}

// extends all words in seqs by all input symbols. Used to generate *all* strings
inline std::vector<word> all_seqs(input min, input max, std::vector<word> const & seqs){
	std::vector<word> ret((max.base() - min.base()) * seqs.size());
	auto it = begin(ret);
	for(auto const & x : seqs){
		for(input i = min; i < max; ++i){
			it->resize(x.size() + 1);
			auto e = copy(x.begin(), x.end(), it->begin());
			*e++ = i;
			it++;
		}
	}
	return ret;
}
