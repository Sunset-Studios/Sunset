#pragma once

#include <minimal.h>
#include <graphics/render_task.h>

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

		private:
			std::vector<RenderTask*> queue;
			RenderTaskExecutor executor;
	};
}
