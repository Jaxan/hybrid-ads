#pragma once

#include <stddef.h> // for size_t
#include <vector>

#ifndef __APPLE__
using uint16_t = __int16;
#endif

// We use size_ts for fast indexing. Note that there is little type safety here
using state = uint16_t;
using input = uint16_t;
using output = uint16_t;

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
	std::vector<word> ret((max - min) * seqs.size());
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
