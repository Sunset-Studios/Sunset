#pragma once

#include <vector>

#include <vk_types.h>
#include <vk_initializers.h>

#include <graphics/pipeline_types.h>
#include <graphics/render_pass_types.h>

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
		void initialize_default_compute(class GraphicsContext* const gfx_context, class Swapchain* const swapchain);
		void initialize_default_graphics(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, const RenderPassConfig& config);
		void destroy(class GraphicsContext* const gfx_context);
		void submit(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer);

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
		void create_default_output_framebuffers(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, const std::initializer_list<ImageID>& attachments);

	protected:
		VulkanRenderPassData data;
		std::vector<PipelineStateID> pass_pipelines_states;
		size_t current_pso_index{ 0 };
	};
}
