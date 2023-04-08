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

	struct IndirectDrawData
	{
		std::vector<IndirectDrawBatch> indirect_draws;
		bool b_needs_refresh{ false };
	};

	struct GPUDrawIndirectBuffers
	{
		BufferID draw_indirect_buffer;
		BufferID object_instance_buffer;
		BufferID compacted_object_instance_buffer;
	};

	struct GPUObjectInstance
	{
		uint32_t object_id;
		uint32_t batch_id;
	};

	struct DrawCullData
	{
		float z_near;
		float z_far;
		float p00;
		float p11;
		float hzb_width;
		float hzb_height;
		uint32_t draw_count;
		uint32_t culling_enabled;
		uint32_t occlusion_enabled;
		uint32_t distance_check;
		int32_t hzb_texture;
		float padding{ 0 };
	};

	struct FullscreenData
	{
		int32_t scene_texture_index;
	};

	struct DepthReduceData
	{
		glm::vec2 reduced_image_size;
		int32_t input_depth_index;
		int32_t output_depth_index;
	};

	struct BloomBlurData
	{
		int32_t input_texture;
		int32_t output_texture;
		glm::vec2 input_texture_size;
		glm::vec2 output_texture_size;
		float bloom_filter_radius;
	};

	struct BloomResolveData
	{
		int32_t scene_color_texure;
		int32_t bloom_brightness_texture;
		float exposure;
		float bloom_intensity;
	};
}