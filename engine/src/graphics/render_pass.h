#pragma once

#include <common.h>
#include <graphics/render_pass_types.h>
#include <graphics/resource/resource_cache.h>

namespace Sunset
{
	template<class Policy>
	class GenericRenderPass
	{
	public:
		GenericRenderPass() = default;

		void initialize_default(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, const RenderPassConfig& config)
		{
			pass_config = config;
			render_pass_policy.initialize_default(gfx_context, swapchain, pass_config);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			render_pass_policy.destroy(gfx_context);
		}

		void* get_data()
		{
			return render_pass_policy.get_data();
		}

		std::vector<class Framebuffer*> get_output_framebuffers()
		{
			return render_pass_policy.get_output_framebuffers();
		}

		void set_output_framebuffers(std::vector<class Framebuffer*>&& framebuffers)
		{
			render_pass_policy.set_output_framebuffers(std::forward<std::vector<class Framebuffer*>>(framebuffers));
		}

		void begin_pass(class GraphicsContext* const gfx_context, uint32_t framebuffer_index, void* command_buffer)
		{
			render_pass_policy.begin_pass(gfx_context, framebuffer_index, command_buffer, pass_config);
		}

		void end_pass(class GraphicsContext* const gfx_context, void* command_buffer)
		{
			render_pass_policy.end_pass(gfx_context, command_buffer, pass_config);
		}

		uint32_t get_num_color_attachments()
		{
			return render_pass_policy.get_num_color_attachments(pass_config);
		}

	private:
		Policy render_pass_policy;
		RenderPassConfig pass_config;
	};

	class NoopRenderPass
	{
	public:
		NoopRenderPass() = default;

		void initialize_default(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RenderPassConfig& config)
		{ }

		void destroy(class GraphicsContext* const gfx_context)
		{ }

		void* get_data()
		{
			return nullptr;
		}

		std::vector<class Framebuffer*> get_output_framebuffers()
		{
			return {};
		}

		void set_output_framebuffers(std::vector<class Framebuffer*>&& framebuffers)
		{ }

		void begin_pass(class GraphicsContext* const gfx_context, uint32_t framebuffer_index, void* command_buffer, const RenderPassConfig& pass_config)
		{ }

		void end_pass(class GraphicsContext* const gfx_context, void* command_buffer, const RenderPassConfig& pass_config)
		{ }

		uint32_t get_num_color_attachments(const RenderPassConfig& config)
		{
			return 0;
		}
	};

#if USE_VULKAN_GRAPHICS
	class RenderPass : public GenericRenderPass<VulkanRenderPass>
	{ };
#else
	class RenderPass : public GenericRenderPass<NoopRenderPass>
	{ };
#endif

	class RenderPassFactory
	{
	public:
		static RenderPassID create_default(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, const RenderPassConfig& config, bool b_auto_delete = false);
	};

	DEFINE_RESOURCE_CACHE(RenderPassCache, RenderPassID, RenderPass);
}
