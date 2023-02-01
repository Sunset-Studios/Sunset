#pragma once

#include <minimal.h>
#include <utility/strings.h>

namespace Sunset
{
	using ImagePathList = std::vector<const char*>;

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
		glm::vec3 extent;
		ImageFlags flags;
		MemoryUsageType usage_type;
		SamplerAddressMode sampler_address_mode;
		ImageFilter image_filter;
		uint8_t attachment_clear : 1 = 1;
		uint8_t attachment_stencil_clear : 1 = 0;
		uint8_t has_store_op : 1 = 1;
		uint8_t padding : 5 = 0;
	};
}