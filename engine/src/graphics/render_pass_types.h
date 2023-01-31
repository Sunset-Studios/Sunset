#pragma once

#include <minimal.h>
#include <functional>

namespace Sunset
{
	enum class RenderPassFlags : int32_t
	{
		None = 0x00000000,
		Compute = 0x00000001,
		Main = 0x00000002
	};

	inline RenderPassFlags operator|(RenderPassFlags lhs, RenderPassFlags rhs)
	{
		return static_cast<RenderPassFlags>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
	}

	inline RenderPassFlags operator&(RenderPassFlags lhs, RenderPassFlags rhs)
	{
		return static_cast<RenderPassFlags>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
	}

	inline RenderPassFlags& operator|=(RenderPassFlags& lhs, RenderPassFlags rhs)
	{
		return lhs = lhs | rhs;
	}

	inline RenderPassFlags& operator&=(RenderPassFlags& lhs, RenderPassFlags rhs)
	{
		return lhs = lhs & rhs;
	}

	struct RenderPassConfig
	{
		Identity name;
		RenderPassFlags flags;
		std::vector<ImageID> attachments;
	};
}