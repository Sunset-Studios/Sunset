#pragma once

#include <minimal.h>

#ifndef READABLE_STRINGS
#define READABLE_STRINGS 1
#endif

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

	struct Identity
	{
		uint32_t computed_hash;
#if READABLE_STRINGS
		std::string string;
#endif

		constexpr Identity(uint32_t hash = 0) noexcept
			: computed_hash(hash)
		{ }

		constexpr Identity(const char* s) noexcept
			: computed_hash(0)
		{
			computed_hash = fnvla_32(s, const_strlen(s));
#if READABLE_STRINGS
			string = s;
#endif
		}

		constexpr Identity(const char* s, std::size_t count) noexcept
			: computed_hash(0)
		{
			computed_hash = fnvla_32(s, count);
#if READABLE_STRINGS
			string = s;
#endif
		}

		constexpr Identity(std::string_view s) noexcept
			: computed_hash(0)
		{
			computed_hash = fnvla_32(s.data(), s.size());
#if READABLE_STRINGS
			string = s;
#endif
		}

		Identity(const Identity& other) = default;

		constexpr operator uint32_t() noexcept
		{
			return computed_hash;
		}

		constexpr operator std::string() noexcept
		{
#if READABLE_STRINGS
			return string;
#else
			return "";
#endif
		}

		constexpr bool operator==(const Identity& other) const noexcept
		{
			return computed_hash == other.computed_hash;
		}
	};
}

#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4267)

template<>
struct std::hash<Sunset::Identity>
{
	std::size_t operator()(const Sunset::Identity& id) const
	{
		return id.computed_hash;
	}
};

#pragma warning( pop ) 