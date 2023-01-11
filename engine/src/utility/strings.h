#pragma once

#include <minimal.h>

namespace Sunset
{
	constexpr uint32_t fnvla_32(char const* s, std::size_t count)
	{
		return ((count > 0 ? fnvla_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
	}

	constexpr size_t const_strlen(const char* s)
	{
		size_t size = 0;
		while (s[size])
		{
			size++;
		}
		return size;
	}

	struct StringHash
	{
		uint32_t computed_hash;

		constexpr StringHash(uint32_t hash) noexcept
			: computed_hash(hash)
		{ }

		constexpr StringHash(const char* s) noexcept
			: computed_hash(0)
		{
			computed_hash = fnvla_32(s, const_strlen(s));
		}

		constexpr StringHash(const char* s, std::size_t count) noexcept
			: computed_hash(0)
		{
			computed_hash = fnvla_32(s, count);
		}

		constexpr StringHash(std::string_view s) noexcept
			: computed_hash(0)
		{
			computed_hash = fnvla_32(s.data(), s.size());
		}

		StringHash(const StringHash& other) = default;

		constexpr operator uint32_t() noexcept
		{
			return computed_hash;
		}
	};
}
