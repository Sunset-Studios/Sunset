#pragma once

#include <minimal.h>

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

	struct AttachmentConfig // About 26 bytes per config. Could probably be slimmed down by removing members we don't need frequent access to on the CPU.
	{
		Format format;
		glm::vec3 extent;
		ImageFlags flags;
		MemoryUsageType usage_type;
		SamplerAddressMode sampler_address_mode;
		ImageFilter image_filter;
		uint8_t attachment_clear : 1;
		uint8_t attachment_stencil_clear : 1;
		uint8_t is_main_depth_attachment : 1;
		uint8_t has_store_op : 1;
		uint8_t padding : 4;

		AttachmentConfig()
			: format(Format::Float4x32), extent(glm::vec3(1920.0f, 1080.0f, 1.0f)), flags(ImageFlags::Image2D),
				usage_type(MemoryUsageType::CPUToGPU), sampler_address_mode(SamplerAddressMode::Repeat), image_filter(ImageFilter::Linear),
				attachment_clear(true), attachment_stencil_clear(false), is_main_depth_attachment(false), has_store_op(true), padding(0)
		{ }
		AttachmentConfig(Format format, const glm::vec3& extent, ImageFlags flags = ImageFlags::None, MemoryUsageType usage_type = MemoryUsageType::CPUToGPU, SamplerAddressMode sampler_address_mode = SamplerAddressMode::Repeat, ImageFilter image_filter = ImageFilter::Linear, bool b_attachment_clear = true, bool b_attachment_stencil_clear = false, bool b_is_main_depth_attachment = false, bool b_has_store_op = true)
			: format(format), extent(extent), flags(flags), usage_type(usage_type), sampler_address_mode(sampler_address_mode), image_filter(image_filter)
		{
			attachment_clear = b_attachment_clear;
			attachment_stencil_clear = b_attachment_stencil_clear;
			is_main_depth_attachment = b_is_main_depth_attachment;
			has_store_op = b_has_store_op;
			padding = 0;
		}
	};
}