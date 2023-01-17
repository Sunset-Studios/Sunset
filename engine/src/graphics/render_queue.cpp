#include <graphics/render_queue.h>
#include <graphics/renderer.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource_state.h>

namespace Sunset
{
	void RenderQueue::add(class RenderTask* const task)
	{
		queue.emplace_back(task);
	}

	void RenderQueue::submit(class GraphicsContext* const gfx_context, void* command_buffer, bool b_flush /*= true*/)
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

		for (const IndirectDrawBatch& draw : indirect_draws)
		{
			execute(gfx_context, draw, command_buffer);
		}

		if (b_flush)
		{
			queue.clear();
		}

		executor.reset();
	}

	void RenderQueue::execute(class GraphicsContext* const gfx_context, RenderTask* const task, void* command_buffer)
	{
		executor(gfx_context, command_buffer, task->material, task->resource_state, task->push_constants);
	}

	void RenderQueue::execute(class GraphicsContext* const gfx_context, const IndirectDrawBatch& indirect_draw, void* command_buffer)
	{
		executor(gfx_context, command_buffer, indirect_draw);
	}

	std::vector<Sunset::IndirectDrawBatch> RenderQueue::batch_indirect_draws(class GraphicsContext* const gfx_context)
	{
		std::vector<IndirectDrawBatch> indirect_draws;

		Buffer* const indirect_buffer = Renderer::get()->indirect_draw_buffer(gfx_context->get_buffered_frame_number());

		{
			ScopedGPUBufferMapping scoped_mapping{ gfx_context, indirect_buffer };

			for (int i = 0; i < queue.size(); ++i)
			{
				RenderTask* const task = queue[i];

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

				ResourceState* const resource_state = ResourceStateCache::get()->fetch(task->resource_state);
				gfx_context->update_indirect_draw_command(
					scoped_mapping.mapped_memory,
					i,
					resource_state->state_data.index_count,
					0,
					1,
					resource_state->state_data.instance_index
				);
			}
		}

		return indirect_draws;
	}
}
