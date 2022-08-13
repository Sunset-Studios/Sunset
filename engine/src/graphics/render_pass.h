#pragma once

#include <common.h>

namespace Sunset
{
	template<class Policy>
	class GenericRenderPass
	{
	public:
		GenericRenderPass() = default;

		void initialize(class GraphicsContext* const gfx_context, class Swapchain* const swapchain)
		{
			render_pass_policy.initialize(gfx_context, swapchain);
		}

		void initialize_default(class GraphicsContext* const gfx_context, class Swapchain* const swapchain)
		{
			render_pass_policy.initialize_default(gfx_context, swapchain);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			render_pass_policy.destroy(gfx_context);
		}

		std::vector<class Framebuffer*> get_output_framebuffers()
		{
			return render_pass_policy.get_output_framebuffers();
		}

		void set_output_framebuffers(std::vector<class Framebuffer*>&& framebuffers)
		{
			render_pass_policy.set_output_framebuffer(std::forward<std::vector<class Framebuffer*>>(framebuffers));
		}

		void begin_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer)
		{
			render_pass_policy.begin_pass(gfx_context, swapchain, command_buffer);
		}

		void end_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer)
		{
			render_pass_policy.end_pass(gfx_context, swapchain, command_buffer);
		}

	private:
		Policy render_pass_policy;
	};

	class NoopRenderPass
	{
	public:
		NoopRenderPass() = default;

		void initialize(class GraphicsContext* const gfx_context, class Swapchain* const swapchain)
		{ }

		void initialize_default(class GraphicsContext* const gfx_context, class Swapchain* const swapchain)
		{ }

		void destroy(class GraphicsContext* const gfx_context)
		{ }

		std::vector<class Framebuffer*> get_output_framebuffers()
		{
			return {};
		}

		void set_output_framebuffers(std::vector<class Framebuffer*>&& framebuffers)
		{ }

		void begin_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer)
		{ }

		void end_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer)
		{ }
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
		template<typename ...Args>
		static RenderPass* create(Args&&... args)
		{
			RenderPass* gfx = new RenderPass;
			gfx->initialize(std::forward<Args>(args)...);
			return gfx;
		}

		template<typename ...Args>
		static RenderPass* create_default(Args&&... args)
		{
			RenderPass* gfx = new RenderPass;
			gfx->initialize_default(std::forward<Args>(args)...);
			return gfx;
		}
	};
}
