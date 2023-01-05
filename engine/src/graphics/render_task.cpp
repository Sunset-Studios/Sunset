#include <graphics/render_task.h>
#include <graphics/render_pass.h>
#include <graphics/renderer.h>
#include <graphics/pipeline_state.h>
#include <graphics/resource_state.h>
#include <graphics/graphics_context.h>
#include <graphics/descriptor.h>

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

	void RenderTaskExecutor::operator()(GraphicsContext* const gfx_context, void* command_buffer, PipelineStateID pipeline_state, ResourceStateID resource_state, const DrawCall& draw_call, const PushConstantPipelineData& push_constants, DescriptorData descriptor_datas[MAX_BOUND_DESCRIPTOR_SETS])
	{
		bool b_pipeline_changed = false;
		if (cached_pipeline_state != pipeline_state)
		{
			PipelineStateCache::get()->fetch(pipeline_state)->bind(gfx_context, command_buffer);
			cached_pipeline_state = pipeline_state;
			b_pipeline_changed = true;
		}

		// TODO: Given that most of our resources will go through descriptors, this resource state will likely get deprecated.
		// Only using it to store vertex buffer info at the moment, but this can be moved to a global merged vertex descriptor buffer
		// that we can index from the vertex shader using some push constant object ID.
		if (cached_resource_state != resource_state)
		{
			ResourceStateCache::get()->fetch(resource_state)->bind(gfx_context, command_buffer);
			cached_resource_state = resource_state;
		}

		for (int i = 0; i < MAX_BOUND_DESCRIPTOR_SETS; ++i)
		{
			if (descriptor_datas[i].descriptor_set != nullptr && (b_pipeline_changed || cached_descriptor_datas[i].descriptor_set != descriptor_datas[i].descriptor_set))
			{
				descriptor_datas[i].descriptor_set->bind(gfx_context, command_buffer, pipeline_state, descriptor_datas[i].dynamic_buffer_offsets, i);
				cached_descriptor_datas[i].descriptor_set = descriptor_datas[i].descriptor_set;
			}
		}

		if (push_constants.data != nullptr)
		{
			gfx_context->push_constants(command_buffer, pipeline_state, push_constants);
		}

		gfx_context->draw(command_buffer, static_cast<uint32_t>(draw_call.vertex_count), 1, ResourceStateCache::get()->fetch(resource_state)->state_data.instance_index);
	}
}
