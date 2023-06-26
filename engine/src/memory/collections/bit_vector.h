#pragma once

#include <vector>
#include <stack_allocator.h>

namespace Sunset
{
	template<size_t Size = 0>
	class BitVector
	{
	public:
		BitVector()
		{
			bits.fill(0);
		}
		BitVector(bool b_no_fill)
		{ }

		~BitVector() = default;

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
			bits.fill(0);
		}

		BitVector operator&(const BitVector& rhs) const
		{
			assert(rhs.size <= size && "Cannot use & bit-wise operator with right hand side BitVector that is smaller in size!");
			BitVector<Size> result(true);
			for (uint32_t i = 0; i < size; ++i)
			{
				result.bits[i] = rhs.bits[i] & bits[i];
			}
			return result;
		}

		BitVector operator|(const BitVector& rhs) const
		{
			assert(rhs.size <= size && "Cannot use | bit-wise operator with right hand side BitVector that is smaller in size!");
			BitVector<Size> result(true);
			for (uint32_t i = 0; i < size; ++i)
			{
				result.bits[i] = rhs.bits[i] | bits[i];
			}
			return result;
		}

		bool operator!=(const BitVector& rhs) const
		{
			return rhs.bits != bits;
		}

		bool operator==(const BitVector& rhs) const
		{
			return rhs.bits == bits;
		}

	private:
		size_t size = Size;
		std::array<size_t, Size> bits;
	};
}