#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <graphics/pipeline_state.h>
#include <graphics/resource_state.h>
#include <graphics/descriptor.h>
#include <window/window.h>

namespace Sunset
{
	void Renderer::setup(Window* const window)
	{
		graphics_context = GraphicsContextFactory::create(window);

		graphics_context->set_buffer_allocator(BufferAllocatorFactory::create(graphics_context.get()));

		graphics_context->register_command_queue(DeviceQueueType::Graphics);
		graphics_context->register_command_queue(DeviceQueueType::Compute);

		swapchain = SwapchainFactory::create(graphics_context.get());

		// TODO: Since a descriptor set allocator is tied to a pool, may want to move this out of here
		// into something more appropriate (e.g. the compute queue would utilize it's own pool)
		graphics_context->set_descriptor_set_allocator(DescriptorSetAllocatorFactory::create(graphics_context.get()));
		graphics_context->get_descriptor_set_allocator()->configure_pool_sizes({
			{ DescriptorType::UniformBuffer, MAX_DESCRIPTOR_BINDINGS },
			{ DescriptorType::DynamicUniformBuffer, MAX_DESCRIPTOR_BINDINGS },
			{ DescriptorType::StorageBuffer, MAX_DESCRIPTOR_BINDINGS },
			{ DescriptorType::Image, MAX_DESCRIPTOR_BINDINGS }
		});

		render_graph.initialize(graphics_context.get());
	}

	void Renderer::destroy()
	{
		for (int16_t frame_number = 0; frame_number < MAX_BUFFERED_FRAMES; ++frame_number)
		{
			graphics_context->wait_for_gpu();
			graphics_context->advance_frame();
		}

		render_graph.destroy(graphics_context.get());

		PipelineStateCache::get()->destroy(graphics_context.get());
		ResourceStateCache::get()->destroy(graphics_context.get());

		graphics_context->destroy_command_queue(DeviceQueueType::Compute);
		graphics_context->destroy_command_queue(DeviceQueueType::Graphics);

		swapchain->destroy(graphics_context.get());

		graphics_context->destroy();
	}

	Sunset::RenderGraph& Renderer::get_render_graph()
	{
		return render_graph;
	}

	Sunset::MeshTaskQueue& Renderer::get_mesh_task_queue()
	{
		return mesh_task_queue;
	}

	Sunset::MeshRenderTask* Renderer::fresh_rendertask()
	{
		return task_allocator.get_new();
	}
}
