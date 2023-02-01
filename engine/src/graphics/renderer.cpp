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
			{ DescriptorType::UniformBuffer, 128 },
			{ DescriptorType::DynamicUniformBuffer, 128 },
			{ DescriptorType::StorageBuffer, 128 },
			{ DescriptorType::Image, 1048 }
			});
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

	Sunset::TaskQueue& Renderer::get_mesh_task_queue()
	{
		return mesh_task_queue;
	}

	Sunset::RenderTask* Renderer::fresh_rendertask()
	{
		return task_allocator.get_new();
	}

	void Renderer::inject_global_descriptor(uint16_t buffered_frame, const std::initializer_list<DescriptorBuildData>& descriptor_build_datas)
	{
		assert(buffered_frame >= 0 && buffered_frame < MAX_BUFFERED_FRAMES);

		DescriptorData& descriptor_data = global_descriptor_data[buffered_frame];

		DescriptorHelpers::inject_descriptors(context(), descriptor_data, descriptor_build_datas);
	}

	void Renderer::inject_object_descriptor(uint16_t buffered_frame, const std::initializer_list<DescriptorBuildData>& descriptor_build_datas)
	{
		assert(buffered_frame >= 0 && buffered_frame < MAX_BUFFERED_FRAMES);

		DescriptorData& descriptor_data = object_descriptor_data[buffered_frame];

		DescriptorHelpers::inject_descriptors(context(), descriptor_data, descriptor_build_datas);
	}
}
