#pragma once

#include <minimal.h>
#include <functional>
#include <utility/strings.h>

namespace Sunset
{
	enum class RenderPassFlags : int32_t
	{
		None = 0x00000000,
		Compute = 0x00000001,
		Graphics = 0x00000002,
		Present = 0x00000004,
		GraphLocal = 0x00000008 // For graph passes that should run locally, without creating and executing an actual GPU pass
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

	struct RenderPassAttachmentInfo
	{
		ImageID image;
		uint32_t image_view_index{ 0 };
	};

	struct RenderPassConfig
	{
		Identity name;
		RenderPassFlags flags;
		std::vector<RenderPassAttachmentInfo> attachments;
		bool b_is_present_pass = false;
	};
}