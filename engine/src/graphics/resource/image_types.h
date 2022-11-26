#pragma once

#include <minimal.h>

namespace Sunset
{
	struct AttachmentConfig // About 20 bytes per config. Could probably be slimmed down by removing members we don't need frequent access to on the CPU.
	{
		Format format;
		glm::vec3 extent;
		ImageFlags flags;
		uint8_t attachment_clear : 1;
		uint8_t attachment_stencil_clear : 1;
		uint8_t is_main_depth_attachment : 1;
		uint8_t has_store_op : 1;

		AttachmentConfig() = default;
		AttachmentConfig(Format format, const glm::vec3& extent, ImageFlags flags = ImageFlags::None, bool b_attachment_clear = true, bool b_attachment_stencil_clear = false, bool b_is_main_depth_attachment = false, bool b_has_store_op = true)
			: format(format), extent(extent), flags(flags)
		{
			attachment_clear = b_attachment_clear;
			attachment_stencil_clear = b_attachment_stencil_clear;
			is_main_depth_attachment = b_is_main_depth_attachment;
			has_store_op = b_has_store_op;
		}
	};
}