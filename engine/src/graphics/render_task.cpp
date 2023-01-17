#include <graphics/render_task.h>
#include <graphics/render_pass.h>
#include <graphics/renderer.h>
#include <graphics/pipeline_state.h>
#include <graphics/resource_state.h>
#include <graphics/graphics_context.h>
#include <graphics/descriptor.h>
#include <graphics/resource/material.h>

namespace Sunset
{
	RenderTask* RenderTask::setup(MaterialID new_material, ResourceStateID new_resource_state, uint32_t new_render_depth /*= 0*/, RenderPassFlags new_render_pass_flags /*= RenderPassFlags::Depth */)
	{
		material = new_material;
		resource_state = new_resource_state;
		render_depth = new_render_depth;
		render_pass_flags = new_render_pass_flags;
		return this;
	}

	void RenderTask::submit(RenderPass* pass)
	{
		pass->push_task(this);
	}

	void RenderTaskExecutor::reset()
	{
		cached_material = 0;
		cached_resource_state = 0;
	}

	void RenderTaskExecutor::operator()(GraphicsContext* const gfx_context, void* command_buffer, MaterialID material, ResourceStateID resource_state, const PushConstantPipelineData& push_constants)
	{
		bool b_pipeline_changed = false;
		if (cached_material != material)
		{
			bind_material_pipeline(gfx_context, command_buffer, material);
			bind_material_descriptors(gfx_context, command_buffer, material);
			cached_material = material;
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

		if (push_constants.data != nullptr)
		{
			gfx_context->push_constants(command_buffer, get_material_pipeline(cached_material), push_constants);
		}

		gfx_context->draw_indexed(
			command_buffer,
			static_cast<uint32_t>(ResourceStateCache::get()->fetch(resource_state)->state_data.index_count),
			1,
			ResourceStateCache::get()->fetch(resource_state)->state_data.instance_index);
	}

	void RenderTaskExecutor::operator()(class GraphicsContext* const gfx_context, void* command_buffer, const IndirectDrawBatch& indirect_draw)
	{
		bool b_pipeline_changed = false;
		if (cached_material != indirect_draw.material)
		{
			bind_material_pipeline(gfx_context, command_buffer, indirect_draw.material);
			bind_material_descriptors(gfx_context, command_buffer, indirect_draw.material);
			cached_material = indirect_draw.material;
			b_pipeline_changed = true;
		}

		// TODO: Given that most of our resources will go through descriptors, this resource state will likely get deprecated.
		// Only using it to store vertex buffer info at the moment, but this can be moved to a global merged vertex descriptor buffer
		// that we can index from the vertex shader using some push constant object ID.
		if (cached_resource_state != indirect_draw.resource_state)
		{
			ResourceStateCache::get()->fetch(indirect_draw.resource_state)->bind(gfx_context, command_buffer);
			cached_resource_state = indirect_draw.resource_state;
		}

		gfx_context->draw_indexed_indirect(
			command_buffer,
			Renderer::get()->indirect_draw_buffer(gfx_context->get_buffered_frame_number()),
			indirect_draw.count,
			indirect_draw.first
		);
	}
}
