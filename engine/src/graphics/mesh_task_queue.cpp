#include <graphics/mesh_task_queue.h>
#include <graphics/renderer.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/mesh.h>
#include <graphics/pipeline_state.h>
#include <graphics/resource_state.h>
#include <graphics/mesh_render_task.h>
#include <utility/cvar.h>

namespace Sunset
{
	AutoCVar_Bool cvar_enable_debug_bounds_draw("ren.enable_debug_bounds_draw", "Whether or not to draw mesh object bounds", false);

	void MeshTaskQueue::sort_and_batch(class GraphicsContext* const gfx_context)
	{
		std::sort(queue.begin(), queue.end(), [](MeshRenderTask* const first, MeshRenderTask* const second) -> bool
		{
			// Sort by pipeline state first (i.e. pipeline settings, blending modes, shaders, etc.)
			// then sort by resource state (i.e. vertex buffer)
			// then sort by world z-depth to minimize overdraw
			return first->material < second->material ||
				(first->material == second->material && first->resource_state < second->resource_state) ||
				(first->resource_state == second->resource_state && first->render_depth < second->render_depth);
		});

		indirect_draw_data.indirect_draws = batch_indirect_draws(gfx_context);
	}

	void MeshTaskQueue::submit_compute_cull(class GraphicsContext* const gfx_context, void* command_buffer, int32_t buffered_frame_number, ExecutionQueue* deletion_queue)
	{
		update_indirect_draw_buffers(gfx_context, command_buffer, buffered_frame_number, deletion_queue);

		gfx_context->dispatch_compute(command_buffer, static_cast<uint32_t>(queue.size() + 255) / 256, 1, 1);

		{
			Buffer* const draw_indirect_buffer = CACHE_FETCH(Buffer, indirect_draw_buffers.draw_indirect_buffer);
			draw_indirect_buffer->barrier(
				gfx_context,
				command_buffer,
				AccessFlags::ShaderRead | AccessFlags::ShaderWrite,
				AccessFlags::IndirectCommandRead,
				PipelineStageType::ComputeShader,
				PipelineStageType::DrawIndirect
			);
		}
	}

	void MeshTaskQueue::submit_draws(class GraphicsContext* const gfx_context, void* command_buffer, RenderPassID render_pass, DescriptorSet* descriptor_set, PipelineStateID pipeline_state, int32_t buffered_frame_number, bool b_use_draw_push_constants /*= true*/, bool b_flush /*= true*/)
	{
		for (uint32_t i = 0; i < indirect_draw_data.indirect_draws.size(); ++i)
		{
			IndirectDrawBatch& draw = indirect_draw_data.indirect_draws[i];

			draw_executor(
				gfx_context,
				command_buffer,
				render_pass,
				descriptor_set,
				draw,
				i,
				CACHE_FETCH(Buffer, indirect_draw_buffers.draw_indirect_buffer),
				pipeline_state,
				buffered_frame_number,
				b_use_draw_push_constants ? draw.push_constants : PushConstantPipelineData()
			);
		}

		if (cvar_enable_debug_bounds_draw.get())
		{
			submit_bounds_debug_draws(gfx_context, command_buffer, render_pass, buffered_frame_number);
		}

		if (b_flush)
		{
			queue.clear();
			draw_executor.reset();
		}
	}

	void MeshTaskQueue::submit_bounds_debug_draws(class GraphicsContext* const gfx_context, void* command_buffer, RenderPassID render_pass, int32_t buffered_frame_number)
	{
		static MeshID sphere_mesh_id = MeshFactory::create_sphere(gfx_context, glm::ivec2(8, 8), 1.0f);
		Mesh* const sphere_mesh = CACHE_FETCH(Mesh, sphere_mesh_id);

		static ResourceStateID sphere_resource_state = ResourceStateBuilder::create()
			.set_vertex_buffer(sphere_mesh->vertex_buffer)
			.set_index_buffer(sphere_mesh->sections[0].index_buffer)
			.set_vertex_count(sphere_mesh->vertices.size())
			.set_index_count(sphere_mesh->sections[0].indices.size())
			.finish();

		static PipelineStateID pipeline_state;
		if (pipeline_state == 0)
		{
			PipelineGraphicsStateBuilder state_builder = PipelineGraphicsStateBuilder::create_default(gfx_context->get_surface_resolution())
				.clear_shader_stages()
				.set_pass(render_pass)
				.value();

			state_builder.set_rasterizer_state(PipelineRasterizerPolygonMode::Line, 1.0f, PipelineRasterizerCullMode::None);
			state_builder.set_attachment_blend_state(PipelineAttachmentBlendState
			{
				.b_blend_enabled = true,
				.source_color_blend = BlendFactor::One,
				.destination_color_blend = BlendFactor::Zero,
				.color_blend_op = BlendOp::Add
			});
			state_builder.set_depth_stencil_state(true, false, CompareOperation::LessOrEqual);
			state_builder.set_shader_stage(PipelineShaderStageType::Vertex, "../../shaders/debug/debug_bounds.vert.sun");
			state_builder.set_shader_stage(PipelineShaderStageType::Fragment, b_is_deferred_rendering ? "../../shaders/debug/debug_bounds_deferred.frag.sun" : "../../shaders/debug/debug_bounds.frag.sun");

			{
				std::vector<DescriptorLayoutID> descriptor_layouts;
				state_builder.derive_shader_layout(descriptor_layouts);
			}

			pipeline_state = state_builder.finish();
		}

		CACHE_FETCH(PipelineState, pipeline_state)->bind(gfx_context, command_buffer);

		for (uint32_t i = 0; i < indirect_draw_data.indirect_draws.size(); ++i)
		{
			IndirectDrawBatch& draw = indirect_draw_data.indirect_draws[i];

			draw.resource_state = sphere_resource_state;

			draw_executor(
				gfx_context,
				command_buffer,
				render_pass,
				nullptr,
				draw,
				i,
				CACHE_FETCH(Buffer, indirect_draw_buffers.draw_indirect_buffer),
				pipeline_state,
				buffered_frame_number
			);
		}
	}

	std::vector<Sunset::IndirectDrawBatch> MeshTaskQueue::batch_indirect_draws(class GraphicsContext* const gfx_context)
	{
		std::vector<IndirectDrawBatch> indirect_draws;

		for (int i = 0; i < queue.size(); ++i)
		{
			MeshRenderTask* const task = queue[i];

			// Indirect draw data buffers are recreated each frame so lets force this refresh for now
			indirect_draw_data.b_needs_refresh = true;

			const bool b_same_resource = !indirect_draws.empty() && task->resource_state == indirect_draws.back().resource_state;
			const bool b_same_material = !indirect_draws.empty() && task->material == indirect_draws.back().material;

			if (b_same_resource && b_same_material)
			{
				indirect_draws.back().count++;
			}
			else
			{
				ResourceState* const resource_state = CACHE_FETCH(ResourceState, task->resource_state);

				IndirectDrawBatch& new_draw_batch = indirect_draws.emplace_back();
				new_draw_batch.resource_state = task->resource_state;
				new_draw_batch.material = task->material;
				new_draw_batch.push_constants = task->push_constants;
				new_draw_batch.first = i;
				new_draw_batch.count = 1;
				new_draw_batch.section_index_count = resource_state->state_data.index_count;
				new_draw_batch.section_index_start = 0;
			}
		}

		return indirect_draws;
	}

	void MeshTaskQueue::update_indirect_draw_buffers(class GraphicsContext* const gfx_context, void* command_buffer, int32_t buffered_frame_number, ExecutionQueue* deletion_queue)
	{
		// This refresh check is unnecessary given that we are now recreating these buffers each frame in order to (hopefully) alias some memory
		// later on as part of each render graph run, but leaving them here in case we decide to make these buffers persistent in the future
		if (indirect_draw_data.b_needs_refresh)
		{
			// Re-upload draw indirect buffer data to GPU with cleared draw count
			Buffer* const draw_indirect_buffer = CACHE_FETCH(Buffer, indirect_draw_buffers.draw_indirect_buffer);
			{
				ScopedGPUBufferMapping scoped_mapping(gfx_context, draw_indirect_buffer);
				
				for (int i = 0; i < indirect_draw_data.indirect_draws.size(); ++i)
				{
					gfx_context->update_indirect_draw_command(
						scoped_mapping.mapped_memory,
						i,
						indirect_draw_data.indirect_draws[i].section_index_count,
						indirect_draw_data.indirect_draws[i].section_index_start,
						0,
						indirect_draw_data.indirect_draws[i].first
					);
				}
			}

			draw_indirect_buffer->barrier(
				gfx_context,
				command_buffer,
				AccessFlags::HostWrite,
				AccessFlags::ShaderRead | AccessFlags::ShaderWrite,
				PipelineStageType::Host,
				PipelineStageType::ComputeShader
			);

			// Re-upload object instance buffer data to GPU
			{
				const BufferID staging_buffer = BufferFactory::create(
					gfx_context,
					{
						.name = "staging_object_instance_buffer" + buffered_frame_number,
						.buffer_size = queue.size() * sizeof(GPUObjectInstance),
						.type = BufferType::TransferSource | BufferType::StorageBuffer,
						.memory_usage = MemoryUsageType::CPUToGPU
					},
					false
				);
				
				if (deletion_queue != nullptr)
				{
					deletion_queue->push_execution([gfx_context, staging_buffer]()
						{
							CACHE_DELETE(Buffer, staging_buffer, gfx_context);
						}
					);
				}

				{
					ScopedGPUBufferMapping scoped_mapping(gfx_context, CACHE_FETCH(Buffer, staging_buffer));

					GPUObjectInstance* object_data = static_cast<GPUObjectInstance*>((void*)scoped_mapping.mapped_memory);

					int32_t object_data_idx = 0;
					for (int i = 0; i < indirect_draw_data.indirect_draws.size(); ++i)
					{
						IndirectDrawBatch batch = indirect_draw_data.indirect_draws[i];
						for (int b = 0; b < batch.count; ++b)
						{
							object_data[object_data_idx].object_id = queue[batch.first + b]->entity;
							object_data[object_data_idx].material_id = queue[batch.first + b]->material_index;
							object_data[object_data_idx].batch_id = i;
							++object_data_idx;
						}
					}
				}

				Buffer* const object_instance_buffer = CACHE_FETCH(Buffer, indirect_draw_buffers.object_instance_buffer);

				object_instance_buffer->copy_buffer(gfx_context, command_buffer, CACHE_FETCH(Buffer, staging_buffer), queue.size() * sizeof(GPUObjectInstance));

				object_instance_buffer->barrier(
					gfx_context,
					command_buffer,
					AccessFlags::TransferWrite,
					AccessFlags::ShaderRead | AccessFlags::ShaderWrite,
					PipelineStageType::Transfer,
					PipelineStageType::ComputeShader
				);
			}

			indirect_draw_data.b_needs_refresh = false;
		}
	}

	void MeshRenderTaskExecutor::operator()(
		GraphicsContext* const gfx_context,
		void* command_buffer,
		RenderPassID render_pass,
		ResourceStateID resource_state,
		PipelineStateID pipeline_state,
		int32_t buffered_frame_number,
		uint32_t instance_count,
		const PushConstantPipelineData& push_constants)
	{
		// TODO: Given that most of our resources will go through descriptors, this resource state will likely get deprecated.
		// Only using it to store vertex buffer info at the moment, but this can be moved to a global merged vertex descriptor buffer
		// that we can index from the vertex shader using some push constant object ID.
		if (cached_resource_state != resource_state)
		{
			CACHE_FETCH(ResourceState, resource_state)->bind(gfx_context, command_buffer);
			cached_resource_state = resource_state;
		}

		if (push_constants.data != nullptr)
		{
			gfx_context->push_constants(command_buffer, pipeline_state, push_constants);
		}

		gfx_context->draw_indexed(
			command_buffer,
			static_cast<uint32_t>(CACHE_FETCH(ResourceState, resource_state)->state_data.index_count),
			instance_count,
			0);
	}

	void MeshRenderTaskExecutor::operator()(
		class GraphicsContext* const gfx_context,
		void* command_buffer,
		RenderPassID render_pass,
		DescriptorSet* descriptor_set,
		const IndirectDrawBatch& indirect_draw,
		uint32_t indirect_draw_index,
		class Buffer* indirect_buffer,
		PipelineStateID pipeline_state,
		int32_t buffered_frame_number,
		const PushConstantPipelineData& push_constants)
	{
		if (cached_material != indirect_draw.material)
		{
			cached_material = indirect_draw.material;
		}

		if (descriptor_set != nullptr)
		{
			material_update(gfx_context, cached_material, descriptor_set, buffered_frame_number);
		}

		// TODO: Given that most of our resources will go through descriptors, this resource state will likely get deprecated.
		// Only using it to store vertex buffer info at the moment, but this can be moved to a global merged vertex descriptor buffer
		// that we can index from the vertex shader using some push constant object ID.
		if (cached_resource_state != indirect_draw.resource_state)
		{
			CACHE_FETCH(ResourceState, indirect_draw.resource_state)->bind(gfx_context, command_buffer);
			cached_resource_state = indirect_draw.resource_state;
		}

		if (push_constants.is_valid())
		{
			gfx_context->push_constants(command_buffer, pipeline_state, push_constants);
		}

		gfx_context->draw_indexed_indirect(
			command_buffer,
			indirect_buffer,
			1,
			indirect_draw_index
		);
	}

	void MeshComputeCullTaskExecutor::operator()(
		class GraphicsContext* const gfx_context,
		void* command_buffer,
		RenderPassID render_pass,
		PipelineStateID pipeline_state,
		const PushConstantPipelineData& push_constants,
		const std::vector<DescriptorLayoutID>& descriptor_layouts)
	{
		if (push_constants.is_valid())
		{
			gfx_context->push_constants(command_buffer, pipeline_state, push_constants);
		}
	}
}
