#pragma once

#include <vk_types.h>
#include <vk_initializers.h>
#include <minimal.h>
#include <graphics/resource/image_types.h>

namespace Sunset
{
	class VulkanImage
	{
	public:
		VulkanImage() = default;

		void initialize(class GraphicsContext* const gfx_context, AttachmentConfig& config);
		void destroy(class GraphicsContext* const gfx_context);
		void copy_from(class GraphicsContext* const gfx_context, void* data);
		void bind(class GraphicsContext* const gfx_context, void* command_buffer);

		void* get_image()
		{
			return image;
		}

		void* get_image_view()
		{
			return image_view;
		}

	protected:
		VkImage image;
		VkImageView image_view;
		VmaAllocation allocation;
	};
}