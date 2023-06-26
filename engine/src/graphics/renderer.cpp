#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/mesh.h>
#include <graphics/resource/image.h>
#include <graphics/pipeline_state.h>
#include <graphics/resource_state.h>
#include <graphics/descriptor.h>
#include <window/window.h>

namespace Sunset
{
	void Renderer::setup(Window* const window, const glm::ivec2& resolution)
	{
		graphics_context = window != nullptr ? GraphicsContextFactory::create(window) : GraphicsContextFactory::create(resolution);
		
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
			wait_for_command_list_build();
			wait_for_gpu();

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

	void Renderer::draw_unit_sphere(void* command_buffer)
	{
		static MeshID unit_sphere_id = MeshFactory::create_sphere(graphics_context.get(), glm::ivec2(32, 32), 1.0f);
		Mesh* const unit_sphere = CACHE_FETCH(Mesh, unit_sphere_id);

		static ResourceStateID unit_sphere_resource_state = ResourceStateBuilder::create()
			.set_vertex_buffer(unit_sphere->vertex_buffer)
			.set_index_buffer(unit_sphere->index_buffer)
			.set_vertex_count(unit_sphere->vertices.size())
			.set_index_count(unit_sphere->indices.size())
			.finish();

		CACHE_FETCH(ResourceState, unit_sphere_resource_state)->bind(graphics_context.get(), command_buffer);

		graphics_context->draw_indexed(
			command_buffer,
			static_cast<uint32_t>(CACHE_FETCH(ResourceState, unit_sphere_resource_state)->state_data.index_count),
			1,
			0);
	}

	void Renderer::draw_unit_cube(void* command_buffer)
	{
		static MeshID unit_cube_id = MeshFactory::create_cube(graphics_context.get());
		Mesh* const unit_cube = CACHE_FETCH(Mesh, unit_cube_id);

		static ResourceStateID unit_cube_resource_state = ResourceStateBuilder::create()
			.set_vertex_buffer(unit_cube->vertex_buffer)
			.set_index_buffer(unit_cube->index_buffer)
			.set_vertex_count(unit_cube->vertices.size())
			.set_index_count(unit_cube->indices.size())
			.finish();

		CACHE_FETCH(ResourceState, unit_cube_resource_state)->bind(graphics_context.get(), command_buffer);

		graphics_context->draw_indexed(
			command_buffer,
			static_cast<uint32_t>(CACHE_FETCH(ResourceState, unit_cube_resource_state)->state_data.index_count),
			1,
			0);
	}

	void Renderer::wait_for_command_list_build()
	{
		ZoneScopedN("ScopedRender::wait_for_command_list_build");
		while (building_command_list[graphics_context->get_buffered_frame_number()].load(std::memory_order_relaxed))
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}

	void Renderer::begin_frame()
	{
		graphics_context->advance_frame();

		wait_for_command_list_build();
		wait_for_gpu();

		task_allocator[graphics_context->get_buffered_frame_number()].reset();

		render_graph.begin(graphics_context.get());
	}

	void Renderer::queue_graph_command(Identity name, std::function<void(class RenderGraph&, RGFrameData&, void*)> command_callback)
	{
		render_graph.add_pass(
			graphics_context.get(),
			name,
			RenderPassFlags::GraphLocal,
			graphics_context->get_buffered_frame_number(),
			command_callback
		);
	}

	void Renderer::register_persistent_image(Identity id, ImageID image)
	{
		persistent_image_map[id] = image;

		// Register copies as well for buffered accesses
		for (uint32_t i = 1; i < MAX_BUFFERED_FRAMES; ++i)
		{
			Identity new_id{ id.computed_hash + i };
			Image* const original = CACHE_FETCH(Image, image);
			AttachmentConfig config = original->get_attachment_config();
			config.name.computed_hash += i;
			persistent_image_map[new_id] = ImageFactory::create(graphics_context.get(), config);
		}
	}

	Sunset::ImageID Renderer::get_persistent_image(Identity id, uint32_t buffered_frame)
	{
		Identity check_id = id.computed_hash + buffered_frame;
		if (persistent_image_map.find(check_id) != persistent_image_map.end())
		{
			return persistent_image_map.at(check_id);
		}
		return ImageID(0);
	}
}
