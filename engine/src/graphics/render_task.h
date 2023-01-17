#pragma once

#include <graphics/pipeline_types.h>
#include <graphics/resource_types.h>
#include <graphics/render_pass_types.h>
#include <graphics/descriptor_types.h>
#include <graphics/renderer_types.h>
#include <graphics/push_constants.h>
#include <memory/allocators/stack_allocator.h>

namespace Sunset
{
	class RenderTaskExecutor
	{
	public:
		RenderTaskExecutor() = default;
		~RenderTaskExecutor() = default;

		void reset();
		void operator()(class GraphicsContext* const gfx_context, void* command_buffer, MaterialID material, ResourceStateID resource_state, const PushConstantPipelineData& push_constants = {});
		void operator()(class GraphicsContext* const gfx_context, void* command_buffer, const IndirectDrawBatch& indirect_draw);

	private:
		MaterialID cached_material{ 0 };
		ResourceStateID cached_resource_state{ 0 };
	};

	class RenderTask
	{
	public:
		RenderTask() = default;
		~RenderTask() = default;

		RenderTask* setup(MaterialID new_material, ResourceStateID new_resource_state, uint32_t new_render_depth = 0, RenderPassFlags new_render_pass_flags = RenderPassFlags::Depth);
		void submit(class RenderPass* pass);

		RenderTask* set_push_constants(PushConstantPipelineData&& push_constant_data)
		{
			push_constants = push_constant_data;
			return this;
		}

	public:
		RenderPassFlags render_pass_flags{ RenderPassFlags::Depth };
		MaterialID material{ 0 };
		ResourceStateID resource_state{ 0 };
		uint32_t render_depth{ 0 };
		PushConstantPipelineData push_constants;
	};

	using RenderTaskFrameAllocator = StaticFrameAllocator<RenderTask>;
}
