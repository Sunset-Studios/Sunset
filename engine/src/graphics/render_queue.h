#pragma once

#include <minimal.h>
#include <graphics/render_task.h>
#include <graphics/resource/material.h>
#include <graphics/renderer_types.h>

namespace Sunset
{
	class RenderQueue
	{
		public:
			RenderQueue() = default;
			~RenderQueue() = default;

			bool empty() const
			{
				return queue.empty();
			}

			void add(RenderTask* const task);
			void submit(class GraphicsContext* const gfx_context, void* command_buffer, bool b_flush = true);

		private:
			void execute(class GraphicsContext* const gfx_context, RenderTask* const task, void* command_buffer);
			void execute(class GraphicsContext* const gfx_context, const IndirectDrawBatch& indirect_draw, void* command_buffer);

			std::vector<IndirectDrawBatch> batch_indirect_draws(class GraphicsContext* const gfx_context);

		private:
			std::vector<RenderTask*> queue;
			RenderTaskExecutor executor;
	};
}
