#pragma once

#include <graphics/pipeline_types.h>
#include <graphics/resource_types.h>
#include <graphics/render_pass_types.h>
#include <memory/allocators/stack_allocator.h>

namespace Sunset
{
	struct DrawCall
	{
		size_t vertex_count{ 0 };
	};

	class RenderTaskExecutor
	{
	public:
		RenderTaskExecutor() = default;
		~RenderTaskExecutor() = default;

		void reset();
		void operator()(class GraphicsContext* const gfx_context, void* command_buffer, PipelineStateID pipeline_state, ResourceStateID resource_state, const DrawCall& draw_call);

	private:
		PipelineStateID cached_pipeline_state{ 0 };
		ResourceStateID cached_resource_state{ 0 };
	};

	class RenderTask
	{
	public:
		RenderTask() = default;
		~RenderTask() = default;

		RenderTask* setup(PipelineStateID new_pipeline_state, ResourceStateID new_resource_state, DrawCall new_draw_call, uint32_t new_render_depth = 0, RenderPassFlags new_render_pass_flags = RenderPassFlags::Depth);
		void submit(class RenderPass* pass);

	public:
		RenderPassFlags render_pass_flags{ RenderPassFlags::Depth };
		PipelineStateID pipeline_state{ 0 };
		ResourceStateID resource_state{ 0 };
		uint32_t render_depth{ 0 };
		DrawCall draw_call;
	};

	using RenderTaskFrameAllocator = StaticFrameAllocator<RenderTask>;
}
