#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/mesh.h>
#include <graphics/resource/swapchain.h>
#include <graphics/resource/image.h>
#include <graphics/pipeline_state.h>
#include <graphics/resource_state.h>
#include <graphics/render_pass.h>
#include <graphics/resource/shader_pipeline_layout.h>
#include <graphics/descriptor.h>

namespace Sunset
{
	void Renderer::setup(Window* const window)
	{
		graphics_window = window;

		graphics_context = GraphicsContextFactory::create(window);
		graphics_context->set_buffer_allocator(BufferAllocatorFactory::create(graphics_context.get()));

		graphics_context->set_descriptor_set_allocator(DescriptorSetAllocatorFactory::create(graphics_context.get()));
		graphics_context->get_descriptor_set_allocator()->configure_pool_sizes({
			{ DescriptorType::UniformBuffer, 128 },
			{ DescriptorType::DynamicUniformBuffer, 128 },
			{ DescriptorType::Image, 1048 }
		});

		swapchain = SwapchainFactory::create(graphics_context.get());
		command_queue = GraphicsCommandQueueFactory::create(graphics_context.get());

		const glm::vec2 image_extent = window->get_extent();
		AttachmentConfig config(Format::FloatDepth32, glm::vec3(image_extent.x, image_extent.y, 1.0f), (ImageFlags::Depth | ImageFlags::Image2D), true, true, true);
		Image* const depth_image = ImageFactory::create(graphics_context.get(), config);

		graphics_master_pass = RenderPassFactory::create_default(graphics_context.get(), swapchain, { depth_image });
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
		for (int16_t frame_number = 0; frame_number < MAX_BUFFERED_FRAMES; ++frame_number)
		{
			graphics_context->wait_for_gpu();
			graphics_context->advance_frame();
		}

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

	void Renderer::inject_global_descriptor(uint16_t buffered_frame, const std::initializer_list<DescriptorBuildData>& descriptor_build_datas)
	{
		assert(buffered_frame >= 0 && buffered_frame < MAX_BUFFERED_FRAMES);

		DescriptorData& descriptor_data = global_descriptor_data[buffered_frame];

		if (descriptor_data.descriptor_set == nullptr)
		{
			DescriptorSetBuilder builder = DescriptorSetBuilder::begin(context());
			for (const DescriptorBuildData& descriptor_build_data : descriptor_build_datas)
			{
				// TODO: Switch on descriptor type to determine whether to bind buffer or image
				builder.bind_buffer(descriptor_build_data);
				
				if (descriptor_build_data.type == DescriptorType::DynamicUniformBuffer)
				{
					descriptor_data.dynamic_buffer_offsets.push_back(descriptor_build_data.buffer_offset);
				}
			}
			builder.build(descriptor_data.descriptor_set, descriptor_data.descriptor_layout);
		}
	}
}
