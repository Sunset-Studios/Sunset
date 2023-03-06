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
		BufferID vertex_buffer{ 0 };
		BufferID index_buffer{ 0 };
		uint32_t vertex_count{ 0 };
		uint32_t index_count{ 0 };

		bool operator==(const ResourceStateData& other) const
		{
			return vertex_buffer == other.vertex_buffer && index_buffer == other.index_buffer 
				&& vertex_count == other.vertex_count && index_count == other.index_count;
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
		std::size_t seed = static_cast<int32_t>(psd.vertex_buffer);
		seed = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(seed), static_cast<int32_t>(psd.index_buffer));
		return seed;
	}
};

#pragma warning( pop ) 