#pragma once

#include <graphics/pipeline_types.h>
#include <graphics/resource_types.h>
#include <graphics/render_pass_types.h>
#include <graphics/push_constants.h>
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
		void operator()(class GraphicsContext* const gfx_context, void* command_buffer, PipelineStateID pipeline_state, ResourceStateID resource_state, const DrawCall& draw_call, const PushConstantPipelineData& push_constants = {}, DescriptorData descriptor_datas[MAX_BOUND_DESCRIPTOR_SETS] = {});

	private:
		PipelineStateID cached_pipeline_state{ 0 };
		ResourceStateID cached_resource_state{ 0 };
		DescriptorData cached_descriptor_datas[MAX_BOUND_DESCRIPTOR_SETS];
	};

	class RenderTask
	{
	public:
		RenderTask() = default;
		~RenderTask() = default;

		RenderTask* setup(PipelineStateID new_pipeline_state, ResourceStateID new_resource_state, DrawCall new_draw_call, uint32_t new_render_depth = 0, RenderPassFlags new_render_pass_flags = RenderPassFlags::Depth);
		void submit(class RenderPass* pass);

		RenderTask* set_push_constants(PushConstantPipelineData&& push_constant_data)
		{
			push_constants = push_constant_data;
			return this;
		}

		RenderTask* set_descriptor_data(DescriptorSetType type, const DescriptorData& descriptor_data)
		{
			uint32_t descriptor_type_idx = static_cast<uint32_t>(type);
			assert(descriptor_type_idx >= 0 && descriptor_type_idx < MAX_BOUND_DESCRIPTOR_SETS);
			descriptor_datas[descriptor_type_idx] = descriptor_data;
			return this;
		}

	public:
		RenderPassFlags render_pass_flags{ RenderPassFlags::Depth };
		PipelineStateID pipeline_state{ 0 };
		ResourceStateID resource_state{ 0 };
		uint32_t render_depth{ 0 };
		DrawCall draw_call;
		PushConstantPipelineData push_constants;
		DescriptorData descriptor_datas[MAX_BOUND_DESCRIPTOR_SETS];
	};

	using RenderTaskFrameAllocator = StaticFrameAllocator<RenderTask>;
}
