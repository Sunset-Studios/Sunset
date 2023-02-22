#pragma once

#include <minimal.h>
#include <graphics/resource_types.h>
#include <graphics/pipeline_types.h>

namespace Sunset
{
	struct IndirectDrawBatch
	{
		MaterialID material;
		ResourceStateID resource_state;
		PushConstantPipelineData push_constants;
		uint32_t first;
		uint32_t count;
	};

	struct GPUDrawIndirectData
	{
		BufferID cleared_draw_indirect_buffer;
		BufferID draw_indirect_buffer;
		BufferID object_instance_buffer;
		BufferID compacted_object_instance_buffer;
		std::vector<IndirectDrawBatch> indirect_draws;
		bool b_needs_refresh{ false };
	};

	struct GPUObjectInstance
	{
		uint32_t object_id;
		uint32_t batch_id;
	};

	struct DrawCullData
	{
		glm::mat4 view;
		float p00, p11, z_near, z_far;
		float frustum[4];
		uint32_t draw_count;
		uint8_t culling_enabled;
		uint8_t occlusion_enabled;
		uint8_t distance_check;
	};
}