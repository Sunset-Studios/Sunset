#pragma once

#include <vector>
#include <functional>

namespace Sunset
{
	using ResourceStateID = size_t;

	struct ResourceStateData
	{
	public:
		class Buffer* vertex_buffer{ nullptr };

		bool operator==(const ResourceStateData& other) const
		{
			return vertex_buffer == other.vertex_buffer;
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
		std::size_t vb_seed = reinterpret_cast<uintptr_t>(psd.vertex_buffer);

		return vb_seed;
	}
};

#pragma warning( pop ) 