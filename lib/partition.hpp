#pragma once

#include <list>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

using Elements = std::list<size_t>;

struct Block {
	Elements::iterator first;
	Elements::iterator second;

	auto begin() const { return first; }
	auto end() const { return std::next(second); }

	bool operator==(Block const & rh) const {
		return first == rh.first && second == rh.second;
	}
};

struct partition_refine {
	using Blocks = std::list<Block>;
	using BlockRef = Blocks::iterator;

	partition_refine(size_t n)
	: elements(n, 0)
	, blocks{{elements.begin(), --elements.end()}}
	{
		std::iota(elements.begin(), elements.end(), 0);
	}

	partition_refine(Block b)
	: elements(b.begin(), b.end())
	, blocks{{elements.begin(), --elements.end()}}
	{}

	partition_refine() = delete;
	partition_refine(partition_refine const &) = delete;
	partition_refine& operator=(partition_refine const &) = delete;
	partition_refine(partition_refine&&) = default;
	partition_refine& operator=(partition_refine&&) = default;

	auto size() const { return blocks.size(); }
	auto begin() { return blocks.begin(); }
	auto end() { return blocks.end(); }

	// Deprecated since there is no more O(1) lookup
	auto find(size_t x){
		for(auto & b : blocks){
			auto it = std::find(b.begin(), b.end(), x);
			if(it != b.begin()) return b;
		}

		return Block{};
	}

	template <typename F>
	auto refine(Block br, F && function, size_t output_size){
		static_assert(std::is_same<decltype(function(0)), size_t>::value, "Function should return size_t");
		if(br == Block{}) throw std::logic_error("Empty block range");
		if(br.first == elements.end() || br.second == elements.end()) throw std::logic_error("Invalid block range");

		Blocks new_blocks;
		std::vector<BlockRef> A(output_size);

		auto it = br.begin();
		auto ed = br.end();
		while(it != ed){
			const auto y = function(*it);
			if(y >= output_size) throw std::runtime_error("Output is too big");

			auto & ar = A[y];
			if(ar == BlockRef{}){
				ar = new_blocks.insert(new_blocks.end(), {it, it});
				it++;
			} else {
				auto current = it++;
				elements.splice(++ar->second, elements, current);
				*ar = {ar->first, current};
			}
		}

		return new_blocks;
	}

	auto replace(BlockRef br, Blocks new_blocks){
		const auto it = blocks.erase(br);
		const auto b = new_blocks.begin();
		const auto e = std::prev(new_blocks.end());
		blocks.splice(it, new_blocks);
		return make_pair(b, std::next(e));
	}

private:
	Elements elements;
	Blocks blocks;
};
