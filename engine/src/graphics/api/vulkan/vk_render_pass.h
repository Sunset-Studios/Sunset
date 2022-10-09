#pragma once

#include <vector>

#include <vk_types.h>
#include <vk_initializers.h>

namespace Sunset
{
	struct VulkanRenderPassData
	{
		VkRenderPass render_pass{ nullptr };
		std::vector<class Framebuffer*> output_framebuffers;
	};

	class VulkanRenderPass
	{
	public:
		VulkanRenderPass() = default;
		~VulkanRenderPass() = default;

	public:
		void initialize(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, std::initializer_list<class PipelineState*> pipelines_states = {});
		void initialize_default(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, std::initializer_list<class PipelineState*> pipelines_states = {});
		void destroy(class GraphicsContext* const gfx_context);
		void draw(class GraphicsContext* const gfx_context, void* command_buffer);

		void* get_data()
		{
			return &data;
		}

		std::vector<class Framebuffer*> get_output_framebuffers()
		{
			return data.output_framebuffers;
		}

		void set_output_framebuffers(std::vector<class Framebuffer*>&& framebuffers)
		{
			data.output_framebuffers = framebuffers;
		}

		void begin_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer);
		void end_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer);

	protected:
		void create_default_output_framebuffers(class GraphicsContext* const gfx_context, class Swapchain* const swapchain);

	protected:
		VulkanRenderPassData data;
		size_t pso_index{ 0 };
	};
}
