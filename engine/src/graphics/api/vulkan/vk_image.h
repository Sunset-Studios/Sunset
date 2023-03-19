#pragma once

#include <vk_types.h>
#include <minimal.h>
#include <graphics/resource/image_types.h>

namespace Sunset
{
	class VulkanImage
	{
	public:
		VulkanImage() = default;

		void initialize(class GraphicsContext* const gfx_context, AttachmentConfig& config);
		void initialize(class GraphicsContext* const gfx_context, const AttachmentConfig& config, void* image_handle, void* image_view_handle);
		void destroy(class GraphicsContext* const gfx_context);
		void copy_buffer(class GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, class Buffer* buffer);
		void bind(class GraphicsContext* const gfx_context, void* command_buffer);

		void* get_image()
		{
			return image;
		}

		void* get_image_view()
		{
			return image_view;
		}

		void* get_sampler()
		{
			return sampler;
		}

		AccessFlags get_access_flags() const
		{
			return access_flags;
		}

		ImageLayout get_layout() const
		{
			return layout;
		}

		void set_access_flags(AccessFlags access)
		{
			access_flags = access;
		}

		void set_layout(ImageLayout new_layout)
		{
			layout = new_layout;
		}

	protected:
		VkImage image;
		VkImageView image_view;
		VkSampler sampler;
		AccessFlags access_flags;
		ImageLayout layout;
		VmaAllocation allocation;
		bool b_external_handling{ false };
	};
}
