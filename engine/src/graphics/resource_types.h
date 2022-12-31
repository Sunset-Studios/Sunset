#pragma once

#include <vector>
#include <functional>
#include <utility/maths.h>

namespace Sunset
{
	using ResourceStateID = size_t;

	struct ResourceStateData
	{
	public:
		class Buffer* vertex_buffer{ nullptr };
		uint32_t instance_index{ 0 };

		bool operator==(const ResourceStateData& other) const
		{
			return vertex_buffer == other.vertex_buffer && instance_index == other.instance_index;
		}
	};
}

#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4267)

template<>
struct std::hash<Sunset::ResourceStateData>
{
	std::size_t operator()(const Sunset::ResourceStateData& psd) const
	{
		std::size_t seed = reinterpret_cast<uintptr_t>(psd.vertex_buffer);
		seed = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(seed), psd.instance_index);

		return seed;
	}
};

#pragma warning( pop ) 