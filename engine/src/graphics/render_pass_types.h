#pragma once

#include <minimal.h>
#include <functional>

namespace Sunset
{
	enum class RenderPassFlags : int32_t
	{
		None = 0,
		Depth = 1,
		Shadow = 1 << 1,
		Main = 1 << 2,
		Post = 1 << 3,
		UI = 1 << 4,
		All = 1 << 31
	};

	inline void render_pass_type_visitor(const std::function<void(RenderPassFlags)>& visitor)
	{
		visitor(RenderPassFlags::Depth);
		visitor(RenderPassFlags::Shadow);
		visitor(RenderPassFlags::Main);
		visitor(RenderPassFlags::Post);
		visitor(RenderPassFlags::UI);
	}
}