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
		void operator()(class GraphicsContext* const gfx_context, void* command_buffer, const IndirectDrawBatch& indirect_draw, class Buffer* indirect_buffer);

	private:
		MaterialID cached_material{ 0 };
		ResourceStateID cached_resource_state{ 0 };
	};

	class RenderTask
	{
	public:
		RenderTask() = default;
		~RenderTask() = default;

		RenderTask* setup(MaterialID new_material, ResourceStateID new_resource_state, uint32_t new_render_depth = 0);
		void submit(TaskQueue& queue);

		RenderTask* set_push_constants(PushConstantPipelineData&& push_constant_data)
		{
			push_constants = push_constant_data;
			return this;
		}

		RenderTask* set_entity(EntityID entity_id)
		{
			entity = entity_id;
			return this;
		}

	public:
		std::size_t task_hash{ 0 };

		EntityID entity{ 0 };
		MaterialID material{ 0 };
		ResourceStateID resource_state{ 0 };
		uint32_t render_depth{ 0 };
		PushConstantPipelineData push_constants;
	};

	using RenderTaskFrameAllocator = StaticFrameAllocator<RenderTask>;
}


#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4267)

template<>
struct std::hash<Sunset::RenderTask>
{
	std::size_t operator()(const Sunset::RenderTask& psd) const
	{
		std::size_t final_hash = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(psd.entity), static_cast<int32_t>(psd.material));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(psd.resource_state));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, psd.render_depth);
		return final_hash;
	}
};

#pragma warning( pop ) 