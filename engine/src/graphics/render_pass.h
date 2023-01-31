#pragma once

#include <common.h>
#include <graphics/pipeline_state.h>
#include <graphics/render_task.h>
#include <graphics/task_queue.h>
#include <graphics/render_pass_types.h>
#include <graphics/resource/resource_cache.h>

namespace Sunset
{
	template<class Policy>
	class GenericRenderPass
	{
	public:
		GenericRenderPass() = default;

		void initialize_default_compute(class GraphicsContext* const gfx_context, class Swapchain* const swapchain)
		{
			render_pass_policy.initialize_default_compute(gfx_context, swapchain);
		}

		void initialize_default_graphics(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, const RenderPassConfig& config)
		{
			pass_config = config;
			render_pass_policy.initialize_default_graphics(gfx_context, swapchain, pass_config);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			render_pass_policy.destroy(gfx_context);
		}

		void submit(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer)
		{
			render_pass_policy.submit(gfx_context, command_buffer);
			task_queue.submit(gfx_context, command_buffer);
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
			task_queue.add(task);
		}

	private:
		Policy render_pass_policy;
		RenderPassConfig pass_config;
		// TODO: Move task queue out and rename to something more akin to it's purpose since it's
		// mostly only used for mesh/object render tasks
		TaskQueue task_queue;
	};

	class NoopRenderPass
	{
	public:
		NoopRenderPass() = default;

		void initialize_default_compute(class GraphicsContext* const gfx_context, class Swapchain* const swapchain)
		{ }

		void initialize_default_graphics(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, const RenderPassConfig& config)
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
		static RenderPass* create_default_compute(class GraphicsContext* const gfx_context, class Swapchain* const swapchain)
		{
			RenderPass* rp = new RenderPass;
			rp->initialize_default_compute(gfx_context, swapchain);
			return rp;
		}

		static RenderPass* create_default_graphics(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, const RenderPassConfig& config)
		{
			RenderPass* rp = new RenderPass;
			rp->initialize_default_graphics(gfx_context, swapchain, config);
			return rp;
		}
	};

	DEFINE_RESOURCE_CACHE(RenderPassCache, RenderPassID, RenderPass);
}
