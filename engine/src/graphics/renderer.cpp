#include <graphics/renderer.h>
#include <window/window.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/swapchain.h>
#include <graphics/command_queue.h>
#include <graphics/render_pass.h>

namespace Sunset
{
	void Renderer::initialize(Window* const window)
	{
		graphics_context = GraphicsContextFactory::create(window);
		swapchain = SwapchainFactory::create(graphics_context);
		command_queue = GraphicsCommandQueueFactory::create(graphics_context);
		render_pass = RenderPassFactory::create_default(graphics_context, swapchain);
	}

	void Renderer::draw()
	{
		graphics_context->wait_for_gpu();

		swapchain->request_next_image(graphics_context);

		void* buffer = command_queue->begin_one_time_buffer_record(graphics_context);

		render_pass->begin_pass(graphics_context, swapchain, buffer);

		// TODO: Bulk of our rendering code here

		render_pass->end_pass(graphics_context, swapchain, buffer);

		command_queue->end_one_time_buffer_record(graphics_context);

		command_queue->submit(graphics_context);

		swapchain->present(graphics_context, command_queue);

		graphics_context->advance_frame();
	}

	void Renderer::destroy()
	{
		command_queue->destroy(graphics_context);
		render_pass->destroy(graphics_context);
		swapchain->destroy(graphics_context);
		graphics_context->destroy();
	}
}
