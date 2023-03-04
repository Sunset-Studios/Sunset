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
		std::vector<FramebufferID> output_framebuffers;
	};

	class VulkanRenderPass
	{
	public:
		VulkanRenderPass() = default;
		~VulkanRenderPass() = default;

	public:
		void initialize_default(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RenderPassConfig& config);
		void destroy(class GraphicsContext* const gfx_context);

		void* get_data()
		{
			return &data;
		}

		std::vector<FramebufferID> get_output_framebuffers()
		{
			return data.output_framebuffers;
		}

		void set_output_framebuffers(std::vector<FramebufferID>&& framebuffers)
		{
			data.output_framebuffers = framebuffers;
		}

		void begin_pass(class GraphicsContext* const gfx_context, uint32_t framebuffer_index, void* command_buffer, const RenderPassConfig& pass_config);
		void end_pass(class GraphicsContext* const gfx_context, void* command_buffer, const RenderPassConfig& pass_config);
		uint32_t get_num_color_attachments(const RenderPassConfig& config);

	protected:
		void create_default_output_framebuffers(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, const RenderPassConfig& config, const std::vector<ImageID>& attachments);

	protected:
		VulkanRenderPassData data;
	};
}
