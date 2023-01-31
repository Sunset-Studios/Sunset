#include <graphics/render_graph.h>
#include <graphics/render_pass.h>
#include <graphics/graphics_context.h>
#include <graphics/command_queue.h>
#ifndef NDEBUG
#include <utility/gui/gui_core.h>
#endif

namespace Sunset
{
	void RenderGraph::initialize(GraphicsContext* const gfx_context)
	{
	}

	void RenderGraph::destroy(GraphicsContext* const gfx_context)
	{
		for (int i = 0; i < registered_passes.size(); ++i)
		{
			if (registered_passes[i] != nullptr)
			{
				registered_passes[i]->destroy(gfx_context);
			}
		}
	}

	void RenderGraph::compile(class GraphicsContext* const gfx_context)
	{
		// TODO: Compute reference counts for resources and cull unreferenced passes
		// TODO: Compute first and last users for non-culled resources
		// TODO: Compute async wait points and resource barriers for non-culled resources
	}

	void RenderGraph::submit(GraphicsContext* const gfx_context, class Swapchain* const swapchain)
	{
		task_allocator.reset();

		// TODO: switch the queue based on the pass type
		void* buffer = gfx_context->get_command_queue(DeviceQueueType::Graphics)->begin_one_time_buffer_record(gfx_context);

		// TODO: Move this into a separate pass outside of the render graph implementation
#ifndef NDEBUG
		global_gui_core.begin_draw();
#endif

		for (RGPassHandle pass_handle : nonculled_passes)
		{
			RenderPass* const pass = registered_passes[pass_handle];
			assert(pass != nullptr);
			pass->begin_pass(gfx_context, swapchain, buffer);
			pass->submit(gfx_context, swapchain, buffer);
			pass->end_pass(gfx_context, swapchain, buffer);
		}

		// TODO: Move this into a separate pass outside of the render graph implementation
#ifndef NDEBUG
		global_gui_core.end_draw(buffer);
#endif
		// TODO: switch the queue based on the pass type
		gfx_context->get_command_queue(DeviceQueueType::Graphics)->end_one_time_buffer_record(gfx_context);
		// TODO: switch the queue based on the pass type
		gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit(gfx_context);
	}

	void RenderGraph::reset()
	{
		
	}
}
