#pragma once

#include <list>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

#include <iostream>

namespace std {
	template <typename Iterator>
	auto begin(pair<Iterator, Iterator> p) { return p.first; }
	template <typename Iterator>
	auto end(pair<Iterator, Iterator> p) { return ++p.second; } // Kind of stupid
}

template <typename R>
size_t elems_in(R range){
	size_t ret = 0;

	auto b = range.first;
	auto e = range.second;
	while(b++ != e) ret++;
	return ret;
}

struct partition_refine {
	using Elements = std::list<size_t>;
	using Block = std::pair<Elements::iterator, Elements::iterator>;
	using Blocks = std::list<Block>;
	using BlockRef = Blocks::iterator;

	partition_refine(size_t n)
	: elements(n, 0)
	, blocks{{begin(elements), --end(elements)}}
	, find_table(n, begin(blocks))
	{
		std::iota(begin(elements), end(elements), 0);
	}

	auto find(size_t element){
		return find_table[element];
	}

	auto size() const {
		return blocks.size();
	}

	template <typename F>
	auto refine(BlockRef br, F && function, size_t output_size){
		static_assert(std::is_same<decltype(function(0)), size_t>::value, "Function should return size_t");
		if(br == BlockRef{}) throw std::logic_error("Passing null ref");
		if(br == blocks.end()) throw std::logic_error("Invalid block");
		if(br->first == elements.end() || br->second == elements.end()) throw std::logic_error("Invalid block range");
		std::vector<BlockRef> A(output_size);

		auto b = *br;
		auto r1 = blocks.erase(br);
		auto r2 = r1;

		auto it = begin(b);
		auto ed = end(b);
		while(it != ed){
			const auto x = *it;
			const auto y = function(x);
			if(y >= output_size) throw std::runtime_error("Output is too big");

			auto & ar = A[y];
			if(ar == BlockRef{}){
				r2 = ar = blocks.insert(r2, {it, it});
				it++;
			} else {
				auto current = it++;
				elements.splice(++ar->second, elements, current);
				*ar = {ar->first, current};
			}
			find_table[x] = ar;
		}

		return make_pair(r2, r1);
	}

private:
	Elements elements;
	std::list<Block> blocks;
	std::vector<BlockRef> find_table;
};
