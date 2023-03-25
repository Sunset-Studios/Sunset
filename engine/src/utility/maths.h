#pragma once

#include <minimal.h>

namespace Sunset
{
	namespace Maths
	{
		inline int64_t cantor_pair_hash(int32_t a, int32_t b)
		{
			const uint64_t shifted_a = (uint64_t)(a >= 0 ? 2 * (int64_t)a : -2 * (int64_t)a - 1);
			const uint64_t shifted_b = (uint64_t)(b >= 0 ? 2 * (int64_t)b : -2 * (int64_t)b - 1);
			const int64_t cantor_pair = (int64_t)((shifted_a >= shifted_b ? shifted_a * shifted_a + shifted_a + shifted_b : shifted_a + shifted_b * shifted_b) / 2);
			return (a < 0) * (b < 0) + (a >= 0) * (b >= 0) ? cantor_pair : -cantor_pair - 1;
		}

		inline uint32_t npot(uint32_t value)
		{
			--value;
			value |= value >> 1;
			value |= value >> 2;
			value |= value >> 4;
			value |= value >> 8;
			value |= value >> 16;
			return ++value;
		}
	};
}
