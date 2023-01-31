#pragma once

#include <minimal.h>
#include <graphics/render_task.h>
#include <graphics/resource/material.h>
#include <graphics/renderer_types.h>

namespace Sunset
{
	class TaskQueue
	{
		public:
			TaskQueue() = default;
			~TaskQueue() = default;

			bool empty() const
			{
				return queue.empty();
			}

			inline void add(RenderTask* const task)
			{
				queue.emplace_back(task);
			}

			void submit(class GraphicsContext* const gfx_context, void* command_buffer, bool b_flush = true);

		private:
			std::vector<IndirectDrawBatch> batch_indirect_draws(class GraphicsContext* const gfx_context);
			void update_indirect_draw_buffers(class GraphicsContext* const gfx_context, void* command_buffer, const std::vector<IndirectDrawBatch>& indirect_batches);

		private:
			std::vector<RenderTask*> queue;
			std::vector<size_t> previous_queue_hashes;
			RenderTaskExecutor executor;
			GPUInstanceIndirectBufferData instance_indirect_buffer_data;
	};
}
