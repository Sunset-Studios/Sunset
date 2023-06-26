#pragma once

#include <core/ecs/entity.h>
#include <graphics/pipeline_types.h>
#include <graphics/resource_types.h>
#include <graphics/render_pass_types.h>
#include <graphics/descriptor_types.h>
#include <graphics/pipeline_types.h>
#include <graphics/mesh_task_queue.h>
#include <memory/allocators/stack_allocator.h>

namespace Sunset
{
	class MeshRenderTask
	{
	public:
		MeshRenderTask() = default;
		~MeshRenderTask() = default;

		MeshRenderTask* setup(MaterialID new_material, ResourceStateID new_resource_state, uint32_t new_render_depth = 0);
		void submit(MeshTaskQueue& queue);

		MeshRenderTask* set_push_constants(PushConstantPipelineData&& push_constant_data)
		{
			push_constants = push_constant_data;
			return this;
		}

		MeshRenderTask* set_entity(EntityID entity_id)
		{
			entity = entity_id;
			return this;
		}

	public:
		EntityID entity{ 0 };
		MaterialID material{ 0 };
		ResourceStateID resource_state{ 0 };
		uint32_t render_depth{ 0 };
		PushConstantPipelineData push_constants;
	};

	using MeshRenderTaskFrameAllocator = StaticFrameAllocator<MeshRenderTask, 65536>;
}


#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4267)

template<>
struct std::hash<Sunset::MeshRenderTask>
{
	std::size_t operator()(const Sunset::MeshRenderTask& psd) const
	{
		std::size_t final_hash = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(psd.entity), static_cast<int32_t>(psd.material));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(psd.resource_state));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, psd.render_depth);
		return final_hash;
	}
};

#pragma warning( pop ) 