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

		graphics_context->set_descriptor_set_allocator(DescriptorSetAllocatorFactory::create(graphics_context.get()));
		graphics_context->get_descriptor_set_allocator()->configure_pool_sizes({
			{ DescriptorType::UniformBuffer, 128 },
			{ DescriptorType::DynamicUniformBuffer, 128 },
			{ DescriptorType::StorageBuffer, 128 },
			{ DescriptorType::Image, 1048 }
			});

		register_pass(
			RenderPassFlags::Compute,
			RenderPassFactory::create_default_compute(graphics_context.get(), swapchain)
		);

		{
			const glm::vec2 image_extent = window->get_extent();
;
			ImageID depth_image = ImageFactory::create(
				graphics_context.get(),
				{
					.name = "main_depth",
					.format = Format::FloatDepth32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = (ImageFlags::Depth | ImageFlags::Image2D),
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::Repeat,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true,
					.attachment_stencil_clear = true,
					.is_main_depth_attachment = true
				}
			);

			register_pass(
				RenderPassFlags::Main,
				RenderPassFactory::create_default_graphics(graphics_context.get(), swapchain, { depth_image })
			);
		}
	}

	void Renderer::draw()
	{
		graphics_context->wait_for_gpu();

		swapchain->request_next_image(graphics_context.get());

		render_graph.submit(graphics_context.get(), swapchain);

		swapchain->present(graphics_context.get(), DeviceQueueType::Graphics);

		graphics_context->advance_frame();
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
	
	class RenderPass* Renderer::pass(RenderPassFlags render_pass)
	{
		for (int i = 0; i < MAX_PASS_COUNT; ++i)
		{
			RenderPassFlags pass_type = static_cast<RenderPassFlags>(1 << i);
			if ((pass_type | render_pass) != RenderPassFlags::None)
			{
				return passes[i];
			}
		}
		return nullptr;
	}

	void Renderer::register_pass(RenderPassFlags pass_type, class RenderPass* render_pass)
	{
		for (int i = 0; i < MAX_PASS_COUNT; ++i)
		{
			RenderPassFlags pass_flag = static_cast<RenderPassFlags>(1 << i);
			if ((pass_flag | pass_type) != RenderPassFlags::None)
			{
				if (passes[i] != nullptr && passes[i] != render_pass)
				{
					passes[i]->destroy(context());
				}
				passes[i] = render_pass;
				return;
			}
		}
	}

	Sunset::RenderGraph& Renderer::get_render_graph()
	{
		return render_graph;
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
