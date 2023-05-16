#pragma once

#include <vk_types.h>
#include <render_pass_types.h>

namespace Sunset
{
	class VulkanFramebuffer
	{
	public:
		VulkanFramebuffer() = default;
		~VulkanFramebuffer() = default;

	public:
		void initialize(class GraphicsContext* const gfx_context, void* render_pass_handle = nullptr, const std::vector<RenderPassAttachmentInfo>& attachments = {});
		void destroy(class GraphicsContext* const gfx_context);
		void* get_framebuffer_handle()
		{
			return framebuffer;
		}
		glm::vec2 get_framebuffer_extent()
		{
			return extent;
		}

	protected:
		VkFramebuffer framebuffer;
		glm::vec2 extent;
	};
}
