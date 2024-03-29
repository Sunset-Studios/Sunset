#pragma once

#include <minimal.h>
#include <utility/strings.h>

namespace Sunset
{
	enum class ImageFilter : uint16_t
	{
		Nearest = 0,
		Linear,
		Cubic
	};

	enum class SamplerAddressMode : uint16_t
	{
		Repeat = 0,
		Mirrored,
		EdgeClamp,
		BorderClamp,
		MirroredEdgeClamp
	};

	struct AttachmentConfig // About 30 bytes per config. Could probably be slimmed down by removing members we don't need frequent access to on the CPU.
	{
		Identity name;
		const char* path;
		Format format;
		glm::vec3 extent{ 0.0f };
		glm::vec3 clear_color{ 0.0f };
		ImageFlags flags;
		MemoryUsageType usage_type;
		SamplerAddressMode sampler_address_mode;
		ImageFilter image_filter;
		uint32_t mip_count{ 1 };
		uint32_t array_count{ 1 };
		uint8_t attachment_clear : 1 = 1;
		uint8_t attachment_stencil_clear : 1 = 0;
		uint8_t has_store_op : 1 = 1;
		uint8_t is_bindless : 1 = 1;
		uint8_t does_min_reduction : 1 = 0;
		uint8_t linear_mip_filtering : 1 = 0;
		uint8_t split_array_layer_views : 1 = 0;
		uint8_t mips_in_rendering: 1 = 1;
		uint8_t padding : 1 = 0;
	};
}