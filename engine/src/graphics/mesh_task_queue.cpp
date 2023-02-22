#include <graphics/mesh_task_queue.h>
#include <graphics/renderer.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource_state.h>
#include <graphics/mesh_render_task.h>

namespace Sunset
{
	void MeshTaskQueue::prepare_batches(class GraphicsContext* const gfx_context)
	{
		sort_and_batch(gfx_context);
		Renderer::get()->get_draw_cull_data().draw_count = queue.size();
	}

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

	void MeshTaskQueue::submit_compute_cull(class GraphicsContext* const gfx_context, void* command_buffer)
	{
		update_indirect_draw_buffers(gfx_context, command_buffer);

		gfx_context->dispatch_compute(command_buffer, queue.size() / 256, 1, 1);

		// Cache off task hashes so we can diff the task queue in the next frame to determine whether we should
		// recompute or re-upload relevant data
		previous_queue_hashes.resize(queue.size(), 0);
		for (int i = 0; i < queue.size(); ++i)
		{
			MeshRenderTask* const task = queue[i];
			previous_queue_hashes[i] = task->task_hash;
		}
	}

	void MeshTaskQueue::submit_draws(class GraphicsContext* const gfx_context, void* command_buffer, RenderPassID render_pass, bool b_flush /*= true*/)
	{
		for (const IndirectDrawBatch& draw : indirect_draw_data.indirect_draws)
		{
			draw_executor(
				gfx_context,
				command_buffer,
				render_pass,
				draw,
				CACHE_FETCH(Buffer, indirect_draw_data.draw_indirect_buffer),
				draw.push_constants
			);
		}

		if (b_flush)
		{
			queue.clear();
		}

		draw_executor.reset();
	}

	std::vector<Sunset::IndirectDrawBatch> MeshTaskQueue::batch_indirect_draws(class GraphicsContext* const gfx_context)
	{
		std::vector<IndirectDrawBatch> indirect_draws;

		const bool b_queue_size_changed = queue.size() != previous_queue_hashes.size();

		for (int i = 0; i < queue.size(); ++i)
		{
			MeshRenderTask* const task = queue[i];

			// Indirect draw data buffers are recreated each frame so lets force this refresh for now
			indirect_draw_data.b_needs_refresh = true; /* b_queue_size_changed || task->task_hash != previous_queue_hashes[i]; */

			const bool b_same_resource = !indirect_draws.empty() && task->resource_state == indirect_draws.back().resource_state;
			const bool b_same_material = !indirect_draws.empty() && task->material == indirect_draws.back().material;

			if (b_same_resource && b_same_material)
			{
				indirect_draws.back().count++;
			}
			else
			{
				IndirectDrawBatch& new_draw_batch = indirect_draws.emplace_back();
				new_draw_batch.resource_state = task->resource_state;
				new_draw_batch.material = task->material;
				new_draw_batch.push_constants = task->push_constants;
				new_draw_batch.first = i;
				new_draw_batch.count = 1;
			}
		}

		return indirect_draws;
	}

	void MeshTaskQueue::update_indirect_draw_buffers(class GraphicsContext* const gfx_context, void* command_buffer)
	{
		// TODO: Batch all barriers in here and only commit them right before pipeline execution

		// A lot of these checks are unnecessary given that we are now recreating these buffers each frame in order to (hopefully) alias some memory
		// later on as part of each render graph run, but leaving them here in case we decide to make these buffers persistent in the future
		if (indirect_draw_data.b_needs_refresh)
		{
			// Re-upload cleared draw indirect buffer data to GPU
			{
				ScopedGPUBufferMapping scoped_mapping(gfx_context, CACHE_FETCH(Buffer, indirect_draw_data.cleared_draw_indirect_buffer));
				
				for (int i = 0; i < indirect_draw_data.indirect_draws.size(); ++i)
				{
					ResourceState* const resource_state = CACHE_FETCH(ResourceState, indirect_draw_data.indirect_draws[i].resource_state);
					gfx_context->update_indirect_draw_command(
						scoped_mapping.mapped_memory,
						i,
						resource_state->state_data.index_count,
						0,
						0,
						resource_state->state_data.instance_index,
						0,
						i
					);
				}
			}
			
			// Copy cleared draw indirect buffer to GPU bound draw indirect buffer
			{
				Buffer* const cleared_draw_indirect_buffer = CACHE_FETCH(Buffer, indirect_draw_data.cleared_draw_indirect_buffer);
				Buffer* const draw_indirect_buffer = CACHE_FETCH(Buffer, indirect_draw_data.draw_indirect_buffer);
				CACHE_FETCH(Buffer, indirect_draw_data.draw_indirect_buffer)->copy_buffer(
					gfx_context,
					command_buffer,
					cleared_draw_indirect_buffer,
					cleared_draw_indirect_buffer->get_size());

				draw_indirect_buffer->barrier(
					gfx_context,
					command_buffer,
					AccessFlags::TransferWrite,
					AccessFlags::ShaderRead | AccessFlags::ShaderWrite,
					PipelineStageType::Transfer,
					PipelineStageType::ComputeShader
				);
			}

			// Re-upload object instance buffer data to GPU
			{
				const BufferID staging_buffer = BufferFactory::create(
					gfx_context,
					{
						.name = "staging_object_instance_buffer",
						.buffer_size = queue.size() * sizeof(GPUObjectInstance),
						.type = BufferType::TransferSource | BufferType::StorageBuffer,
						.memory_usage = MemoryUsageType::CPUToGPU
					},
					false
				);
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
							object_data[object_data_idx].batch_id = i;
							++object_data_idx;
						}
					}
				}

				Buffer* const object_instance_buffer = CACHE_FETCH(Buffer, indirect_draw_data.object_instance_buffer);

				object_instance_buffer->copy_buffer(gfx_context, command_buffer, CACHE_FETCH(Buffer, staging_buffer), queue.size() * sizeof(GPUObjectInstance));
				CACHE_DELETE(Buffer, staging_buffer, gfx_context);

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
		MaterialID material,
		ResourceStateID resource_state,
		const PushConstantPipelineData& push_constants)
	{
		bool b_pipeline_changed = false;
		if (cached_material != material)
		{
			material_setup_pipeline_state(gfx_context, material, render_pass);
			material_bind_pipeline(gfx_context, command_buffer, material);
			material_bind_descriptors(gfx_context, command_buffer, material);
			cached_material = material;
			b_pipeline_changed = true;
		}

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
			gfx_context->push_constants(command_buffer, material_get_pipeline(cached_material), push_constants);
		}

		gfx_context->draw_indexed(
			command_buffer,
			static_cast<uint32_t>(CACHE_FETCH(ResourceState, resource_state)->state_data.index_count),
			1,
			CACHE_FETCH(ResourceState, resource_state)->state_data.instance_index);
	}

	void MeshRenderTaskExecutor::operator()(
		class GraphicsContext* const gfx_context,
		void* command_buffer,
		RenderPassID render_pass,
		const IndirectDrawBatch& indirect_draw,
		class Buffer* indirect_buffer,
		const PushConstantPipelineData& push_constants)
	{
		bool b_pipeline_changed = false;
		if (cached_material != indirect_draw.material)
		{
			material_setup_pipeline_state(gfx_context, indirect_draw.material, render_pass);
			material_bind_pipeline(gfx_context, command_buffer, indirect_draw.material);
			material_bind_descriptors(gfx_context, command_buffer, indirect_draw.material);
			cached_material = indirect_draw.material;
			b_pipeline_changed = true;
		}

		// TODO: Given that most of our resources will go through descriptors, this resource state will likely get deprecated.
		// Only using it to store vertex buffer info at the moment, but this can be moved to a global merged vertex descriptor buffer
		// that we can index from the vertex shader using some push constant object ID.
		if (cached_resource_state != indirect_draw.resource_state)
		{
			CACHE_FETCH(ResourceState, indirect_draw.resource_state)->bind(gfx_context, command_buffer);
			cached_resource_state = indirect_draw.resource_state;
		}

		if (push_constants.data != nullptr)
		{
			gfx_context->push_constants(command_buffer, material_get_pipeline(cached_material), push_constants);
		}

		gfx_context->draw_indexed_indirect(
			command_buffer,
			indirect_buffer,
			indirect_draw.count,
			indirect_draw.first
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
		if (push_constants.data != nullptr)
		{
			gfx_context->push_constants(command_buffer, pipeline_state, push_constants);
		}
	}
}