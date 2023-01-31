#include <graphics/task_queue.h>
#include <graphics/renderer.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource_state.h>

namespace Sunset
{
	void TaskQueue::submit(class GraphicsContext* const gfx_context, void* command_buffer, bool b_flush /*= true*/)
	{
		std::sort(queue.begin(), queue.end(), [](RenderTask* const first, RenderTask* const second) -> bool
		{
			// Sort by pipeline state first (i.e. pipeline settings, blending modes, shaders, etc.)
			// then sort by resource state (i.e. vertex buffer)
			// then sort by world z-depth to minimize overdraw
			return first->material < second->material ||
			(first->material == second->material && first->resource_state < second->resource_state) ||
			(first->resource_state == second->resource_state && first->render_depth < second->render_depth);
		});

		std::vector<IndirectDrawBatch> indirect_draws = batch_indirect_draws(gfx_context);

		// TODO_BEGIN: Compute culling work should happen around here
		update_indirect_draw_buffers(gfx_context, command_buffer, indirect_draws);

		instance_indirect_buffer_data.object_instance_buffer->barrier(
			gfx_context,
			command_buffer,
			AccessFlags::TransferWrite,
			AccessFlags::ShaderRead | AccessFlags::ShaderWrite,
			PipelineStageType::Transfer,
			PipelineStageType::ComputeShader
		);
		// TODO_END: Compute culling work should happen around here

		for (const IndirectDrawBatch& draw : indirect_draws)
		{
			executor(gfx_context, command_buffer, draw, instance_indirect_buffer_data.draw_indirect_buffer);
		}

		// Cache off task hashes so we can diff the task queue in the next frame to determine whether we should
		// recompute or re-upload relevant data
		previous_queue_hashes.resize(queue.size(), 0);
		for (int i = 0; i < queue.size(); ++i)
		{
			RenderTask* const task = queue[i];
			previous_queue_hashes[i] = task->task_hash;
		}

		if (b_flush)
		{
			queue.clear();
		}

		executor.reset();
	}

	std::vector<Sunset::IndirectDrawBatch> TaskQueue::batch_indirect_draws(class GraphicsContext* const gfx_context)
	{
		std::vector<IndirectDrawBatch> indirect_draws;

		const bool b_queue_size_changed = queue.size() != previous_queue_hashes.size();

		for (int i = 0; i < queue.size(); ++i)
		{
			RenderTask* const task = queue[i];

			instance_indirect_buffer_data.b_needs_refresh = b_queue_size_changed || task->task_hash != previous_queue_hashes[i];

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
				new_draw_batch.first = i;
				new_draw_batch.count = 1;
			}
		}

		return indirect_draws;
	}

	void TaskQueue::update_indirect_draw_buffers(class GraphicsContext* const gfx_context, void* command_buffer, const std::vector<IndirectDrawBatch>& indirect_batches)
	{
		// Reallocate GPU-only draw indirect buffer
		if (instance_indirect_buffer_data.draw_indirect_buffer == nullptr)
		{
			instance_indirect_buffer_data.draw_indirect_buffer = BufferFactory::create(
				gfx_context,
				{
					.name = "draw_indirect_buffer",
					.buffer_size = std::max(static_cast<size_t>(256), indirect_batches.size()),
					.type = BufferType::TransferDestination | BufferType::StorageBuffer | BufferType::Indirect,
					.memory_usage = MemoryUsageType::OnlyGPU
				}
			);
		}
		else if (instance_indirect_buffer_data.draw_indirect_buffer->get_size() < indirect_batches.size())
		{
			instance_indirect_buffer_data.draw_indirect_buffer->reallocate(gfx_context, std::max(static_cast<size_t>(256), indirect_batches.size()));
		}

		// Reallocate GPU-only compacted object instance buffer
		if (instance_indirect_buffer_data.compacted_object_instance_buffer == nullptr)
		{
			instance_indirect_buffer_data.compacted_object_instance_buffer = BufferFactory::create(
				gfx_context,
				{
					.name = "compacted_object_instance_buffer",
					.buffer_size = std::max(static_cast<size_t>(256) * sizeof(uint32_t), queue.size() * sizeof(uint32_t)),
					.type = BufferType::TransferDestination | BufferType::StorageBuffer,
					.memory_usage = MemoryUsageType::OnlyGPU
				}
			);
		}
		else if (instance_indirect_buffer_data.compacted_object_instance_buffer->get_size() < queue.size() * sizeof(uint32_t))
		{
			instance_indirect_buffer_data.compacted_object_instance_buffer->reallocate(gfx_context, queue.size() * sizeof(uint32_t));
		}

		// Reallocate GPU-only object instance buffer
		if (instance_indirect_buffer_data.object_instance_buffer == nullptr)
		{
			instance_indirect_buffer_data.object_instance_buffer = BufferFactory::create(
				gfx_context,
				{
					.name = "object_instance_buffer",
					.buffer_size = std::max(static_cast<size_t>(256) * sizeof(GPUObjectInstance), queue.size() * sizeof(GPUObjectInstance)),
					.type = BufferType::TransferDestination | BufferType::StorageBuffer,
					.memory_usage = MemoryUsageType::OnlyGPU
				}
			);
		}
		else if (instance_indirect_buffer_data.object_instance_buffer->get_size() < queue.size() * sizeof(GPUObjectInstance))
		{
			instance_indirect_buffer_data.object_instance_buffer->reallocate(gfx_context, std::max(static_cast<size_t>(256) * sizeof(GPUObjectInstance), queue.size() * sizeof(GPUObjectInstance)));
		}

		if (instance_indirect_buffer_data.b_needs_refresh)
		{
			// Re-upload cleared draw indirect buffer data to GPU
			if (instance_indirect_buffer_data.cleared_draw_indirect_buffer != nullptr)
			{
				instance_indirect_buffer_data.cleared_draw_indirect_buffer = BufferFactory::create(
					gfx_context,
					{
						.name = "cleared_draw_indirect_buffer",
						.buffer_size = indirect_batches.size(),
						.type = BufferType::TransferSource | BufferType::StorageBuffer | BufferType::Indirect,
						.memory_usage = MemoryUsageType::CPUToGPU
					}
				);
			}
			else
			{
				instance_indirect_buffer_data.cleared_draw_indirect_buffer->reallocate(gfx_context, indirect_batches.size());
			}

			{
				ScopedGPUBufferMapping scoped_mapping(gfx_context, instance_indirect_buffer_data.cleared_draw_indirect_buffer);
				
				for (int i = 0; i < indirect_batches.size(); ++i)
				{
					ResourceState* const resource_state = ResourceStateCache::get()->fetch(indirect_batches[i].resource_state);
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

			// Re-upload object instance buffer data to GPU
			{
				Buffer* const staging_buffer = BufferFactory::create(
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
					ScopedGPUBufferMapping scoped_mapping(gfx_context, staging_buffer);

					GPUObjectInstance* object_data = static_cast<GPUObjectInstance*>((void*)scoped_mapping.mapped_memory);

					int32_t object_data_idx = 0;
					for (int i = 0; i < indirect_batches.size(); ++i)
					{
						IndirectDrawBatch batch = indirect_batches[i];
						for (int b = 0; b < batch.count; ++b)
						{
							object_data[object_data_idx].object_id = queue[batch.first + b]->entity;
							object_data[object_data_idx].batch_id = i;
							++object_data_idx;
						}
					}
				}

				instance_indirect_buffer_data.object_instance_buffer->copy_buffer(gfx_context, command_buffer, staging_buffer, queue.size() * sizeof(GPUObjectInstance));
			}

			instance_indirect_buffer_data.b_needs_refresh = false;
		}
	}
}
