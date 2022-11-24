#include <graphics/render_task.h>
#include <graphics/render_pass.h>
#include <graphics/renderer.h>
#include <graphics/pipeline_state.h>
#include <graphics/resource_state.h>
#include <graphics/graphics_context.h>

namespace Sunset
{
	RenderTask* RenderTask::setup(PipelineStateID new_pipeline_state, ResourceStateID new_resource_state, DrawCall new_draw_call, uint32_t new_render_depth /*= 0*/, RenderPassFlags new_render_pass_flags /*= RenderPassFlags::Depth */)
	{
		pipeline_state = new_pipeline_state;
		resource_state = new_resource_state;
		render_depth = new_render_depth;
		draw_call = new_draw_call;
		render_pass_flags = new_render_pass_flags;
		return this;
	}

	void RenderTask::submit(RenderPass* pass)
	{
		pass->push_task(this);
	}

	void RenderTaskExecutor::reset()
	{
		cached_pipeline_state = 0;
		cached_resource_state = 0;
	}

	void RenderTaskExecutor::operator()(GraphicsContext* const gfx_context, void* command_buffer, PipelineStateID pipeline_state, ResourceStateID resource_state, const DrawCall& draw_call, const PushConstantPipelineData& push_constants)
	{
		if (cached_pipeline_state != pipeline_state)
		{
			PipelineStateCache::get()->fetch(pipeline_state)->bind(gfx_context, command_buffer);
			cached_pipeline_state = pipeline_state;
		}

		if (cached_resource_state != resource_state)
		{
			ResourceStateCache::get()->fetch(resource_state)->bind(gfx_context, command_buffer);
			cached_resource_state = resource_state;
		}

		if (push_constants.data != nullptr)
		{
			gfx_context->push_constants(command_buffer, pipeline_state, push_constants);
		}

		gfx_context->draw(command_buffer, static_cast<uint32_t>(draw_call.vertex_count), 1);
	}
}
