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
		uint32_t section_index_start;
		uint32_t section_index_count;
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
		uint32_t material_id;
	};

	struct CompactedGPUObjectInstance
	{
		uint32_t object_id;
		uint32_t material_id;
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

	struct CSMData
	{
		int32_t shadow_cascade;
	};

	struct MotionVectorsData
	{
		int32_t input_depth_index;
		int32_t output_motion_vectors_index;
		glm::vec2 resolution;
	};

	struct FullscreenData
	{
		int32_t scene_texture_index{ -1 };
		int32_t scene_texture_layer{ -1 };
	};

	struct DepthReduceData
	{
		glm::vec2 reduced_image_size;
		int32_t input_depth_index;
		int32_t output_depth_index;
	};

	struct LightingPassData
	{
		int32_t albedo_texure;
		int32_t smra_texure;
		int32_t cc_texture;
		int32_t normal_texure;
		int32_t position_texure;
		int32_t sky_texure;
		int32_t ssao_texture;
		int32_t shadow_texture;
	};

	struct FXAAData
	{
		int32_t input_scene_color;
		int32_t output_scene_color;
		float min_edge_threshold;
		float max_edge_threshold;
		glm::vec2 resolution;
		glm::vec2 inv_resolution;
		int32_t max_iterations;
		glm::vec3 padding;
	};

	struct TAAData
	{
		int32_t input_scene_color;
		int32_t input_color_history;
		int32_t input_motion_vectors;
		int32_t input_depth;
		int32_t output_scene_color;
		int32_t output_color_history;
		int32_t inverse_luminance_filter_enabled;
		int32_t luminance_difference_filter_enabled;
		glm::vec2 resolution;
		glm::vec2 padding;
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

	constexpr uint32_t SSAOKernelSize = 64;

	struct SSAOKernelData
	{
		glm::vec3 kernel[SSAOKernelSize];
	};

	struct SSAOData
	{
		int32_t ssao_noise_texture;
		int32_t position_texture;
		int32_t normal_texture;
		int32_t out_ssao_texture;
		glm::vec2 noise_scale;
		glm::vec2 resolution;
		float radius;
		float bias;
		float strength;
		float padding;
	};

	struct SSRData
	{
		int32_t scene_color_texture;
		int32_t position_texture;
		int32_t normal_texture;
		int32_t smra_texture;
		int32_t out_ssr_texture;
		int32_t ray_hit_steps;
		float max_ray_distance;
		float padding;
		glm::vec2 resolution;
	};

	struct SSRResolveData
	{
		int32_t smra_texture;
		int32_t ssr_texture;
		int32_t ssr_blurred_texture;
		int32_t scene_color_texture;
		float ssr_strength;
	};

	struct BoxBlurData
	{
		int32_t staging_texture;
		int32_t out_texture;
		glm::vec2 resolution;
	};
}