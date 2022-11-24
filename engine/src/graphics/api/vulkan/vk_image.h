#pragma once

#include <vk_types.h>
#include <vk_initializers.h>

namespace Sunset
{
	class VulkanImage
	{
	public:
		VulkanImage() = default;

		void initialize(class GraphicsContext* const gfx_context, Format format, ImageType image_type, const glm::vec3& extent, ImageUsage usage);
		void destroy(class GraphicsContext* const gfx_context);
		void copy_from(class GraphicsContext* const gfx_context, void* data);
		void bind(class GraphicsContext* const gfx_context, void* command_buffer);

	protected:
		VkImage image;
		VkImageView image_view;
		VmaAllocation allocation;
	};
}
