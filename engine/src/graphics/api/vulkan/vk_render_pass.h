#pragma once

#include <vector>

#include <vk_types.h>
#include <vk_initializers.h>

namespace Sunset
{
	class VulkanRenderPass
	{
	public:
		VulkanRenderPass() = default;
		~VulkanRenderPass() = default;

	public:
		void initialize(class GraphicsContext* const gfx_context, class Swapchain* const swapchain);
		void initialize_default(class GraphicsContext* const gfx_context, class Swapchain* const swapchain);
		void destroy(class GraphicsContext* const gfx_context);

		std::vector<class Framebuffer*> get_output_framebuffers()
		{
			return output_framebuffers;
		}

		void set_output_framebuffers(std::vector<class Framebuffer*>&& framebuffers)
		{
			output_framebuffers = framebuffers;
		}

		void begin_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer);
		void end_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer);

	protected:
		void create_default_output_framebuffers(class GraphicsContext* const gfx_context, class Swapchain* const swapchain);

	protected:
		VkRenderPass render_pass;
		std::vector<class Framebuffer*> output_framebuffers;
	};
}
