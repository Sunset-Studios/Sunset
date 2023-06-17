#pragma once

#include <vector>

namespace Sunset
{
	class BitVector
	{
	public:
		BitVector() = default;
		BitVector(size_t size)
			: bits(size, 0)
		{ }

		bool test(size_t bit)
		{
			const size_t bit_index = bit % sizeof(size_t);
			const size_t word_index = bit / sizeof(size_t) + (bit_index != 0);
			return bits[word_index] & (size_t(1) << bit_index);
		}

		void set(size_t bit)
		{
			const size_t bit_index = bit % sizeof(size_t);
			const size_t word_index = bit / sizeof(size_t) + (bit_index != 0);
			bits[word_index] |= (size_t(1) << bit_index);
		}

		void unset(size_t bit)
		{
			const size_t bit_index = bit % sizeof(size_t);
			const size_t word_index = bit / sizeof(size_t) + (bit_index != 0);
			bits[word_index] &= ~(size_t(1) << bit_index);
		}

		void reset()
		{
			bits.clear();
			bits.resize(bits.capacity(), 0);
		}

	private:
		std::vector<size_t> bits;
	};
}