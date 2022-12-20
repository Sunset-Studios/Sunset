#include <graphics/render_queue.h>

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
			return first->pipeline_state < second->pipeline_state ||
			(first->pipeline_state == second->pipeline_state && first->resource_state < second->resource_state) ||
			(first->resource_state == second->resource_state && first->render_depth < second->render_depth);
		});

		for (RenderTask* const task : queue)
		{
			execute(gfx_context, task, command_buffer);
		}

		if (b_flush)
		{
			queue.clear();
		}

		executor.reset();
	}

	void RenderQueue::execute(class GraphicsContext* const gfx_context, RenderTask* const task, void* command_buffer)
	{
		executor(gfx_context, command_buffer, task->pipeline_state, task->resource_state, task->draw_call, task->push_constants, task->descriptor_datas);
	}
}
