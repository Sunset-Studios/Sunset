#pragma once

#include <common.h>
#include <graphics/pipeline_state.h>
#include <graphics/render_task.h>
#include <graphics/render_queue.h>

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

		void draw(class GraphicsContext* const gfx_context, void* command_buffer)
		{
			render_pass_policy.draw(gfx_context, command_buffer);
			render_queue.submit(gfx_context, command_buffer);
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

		void begin_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer)
		{
			render_pass_policy.begin_pass(gfx_context, swapchain, command_buffer);
		}

		void end_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer)
		{
			render_pass_policy.end_pass(gfx_context, swapchain, command_buffer);
		}

		void push_task(RenderTask* task)
		{
			render_queue.add(task);
		}

	private:
		Policy render_pass_policy;
		RenderQueue render_queue;
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

		void draw(class GraphicsContext* const gfx_context)
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
		static RenderPass* create(class GraphicsContext* const gfx_context, class Swapchain* const swapchain)
		{
			RenderPass* rp = new RenderPass;
			rp->initialize(gfx_context, swapchain);
			return rp;
		}

		template<typename ...Args>
		static RenderPass* create_default(class GraphicsContext* const gfx_context, class Swapchain* const swapchain)
		{
			RenderPass* rp = new RenderPass;
			rp->initialize_default(gfx_context, swapchain);
			return rp;
		}
	};
}
