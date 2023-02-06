#pragma once

#include <vk_types.h>
#include <vk_initializers.h>

namespace Sunset
{
	class VulkanFramebuffer
	{
	public:
		VulkanFramebuffer() = default;
		~VulkanFramebuffer() = default;

	public:
		void initialize(class GraphicsContext* const gfx_context, void* render_pass_handle = nullptr, const std::vector<ImageID>& attachments = {});
		void destroy(class GraphicsContext* const gfx_context);
		void* get_framebuffer_handle()
		{
			return framebuffer;
		}

	protected:
		VkFramebuffer framebuffer;
	};
}
