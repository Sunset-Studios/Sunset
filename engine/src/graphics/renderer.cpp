#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/mesh.h>
#include <graphics/resource/swapchain.h>
#include <graphics/pipeline_state.h>
#include <graphics/resource_state.h>
#include <graphics/render_pass.h>
#include <graphics/resource/shader_pipeline_layout.h>

namespace Sunset
{
	void Renderer::setup(Window* const window)
	{
		graphics_window = window;

		graphics_context = GraphicsContextFactory::create(window);
		graphics_context->set_buffer_allocator(BufferAllocatorFactory::create(graphics_context.get()));

		swapchain = SwapchainFactory::create(graphics_context.get());
		command_queue = GraphicsCommandQueueFactory::create(graphics_context.get());

		graphics_master_pass = RenderPassFactory::create_default(graphics_context.get(), swapchain);
	}

	void Renderer::draw()
	{
		graphics_context->wait_for_gpu();

		rendertask_allocator.reset();

		swapchain->request_next_image(graphics_context.get());

		void* buffer = command_queue->begin_one_time_buffer_record(graphics_context.get());

		graphics_master_pass->begin_pass(graphics_context.get(), swapchain, buffer);
		graphics_master_pass->draw(graphics_context.get(), buffer);
		graphics_master_pass->end_pass(graphics_context.get(), swapchain, buffer);

		command_queue->end_one_time_buffer_record(graphics_context.get());

		command_queue->submit(graphics_context.get());

		swapchain->present(graphics_context.get(), command_queue.get());

		graphics_context->advance_frame();
	}

	void Renderer::destroy()
	{
		graphics_context->wait_for_gpu();

		graphics_master_pass->destroy(graphics_context.get());

		PipelineStateCache::get()->destroy(graphics_context.get());
		ResourceStateCache::get()->destroy(graphics_context.get());

		command_queue->destroy(graphics_context.get());

		swapchain->destroy(graphics_context.get());

		graphics_context->destroy();
	}

	RenderTask* Renderer::fresh_rendertask()
	{
		return rendertask_allocator.get_new();
	}
}
