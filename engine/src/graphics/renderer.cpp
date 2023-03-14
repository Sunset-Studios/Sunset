#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/mesh.h>
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

	void Renderer::draw_fullscreen_quad(void* command_buffer)
	{
		static MeshID fullscreen_quad_id = MeshFactory::create_quad(graphics_context.get());
		Mesh* const fullscreen_quad = CACHE_FETCH(Mesh, fullscreen_quad_id);

		static ResourceStateID fullscreen_quad_resource_state = ResourceStateBuilder::create()
			.set_vertex_buffer(fullscreen_quad->vertex_buffer)
			.set_index_buffer(fullscreen_quad->index_buffer)
			.set_vertex_count(fullscreen_quad->vertices.size())
			.set_index_count(fullscreen_quad->indices.size())
			.finish();

		CACHE_FETCH(ResourceState, fullscreen_quad_resource_state)->bind(graphics_context.get(), command_buffer);

		graphics_context->draw_indexed(
			command_buffer,
			static_cast<uint32_t>(CACHE_FETCH(ResourceState, fullscreen_quad_resource_state)->state_data.index_count),
			1,
			0);
	}

	void Renderer::begin_frame()
	{
		render_graph.begin(graphics_context.get());
	}

	void Renderer::queue_graph_command(Identity name, std::function<void(class RenderGraph&, RGFrameData&, void*)> command_callback)
	{
		render_graph.add_pass(
			graphics_context.get(),
			name,
			RenderPassFlags::GraphLocal,
			command_callback
		);
	}
}
