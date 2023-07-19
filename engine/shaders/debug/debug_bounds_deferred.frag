#version 460

#include "common/common.inc"
#include "common/lighting_common.inc"

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) flat in uint in_instance_index;

layout (location = 0) out vec4 out_frag_color;
layout (location = 1) out float out_specular;
layout (location = 2) out vec4 out_normal;
layout (location = 3) out vec4 out_position;

struct EntitySceneData
{
	mat4 transform;
	vec4 bounds_pos_radius;
	vec4 bounds_extent;
};

struct CompactedObjectInstance
{
	uint object_id;
	uint material_id;
};

struct MaterialData
{
	int textures[MAX_TEXTURES_PER_MATERIAL];
	float tiling_coeffs[MAX_TEXTURES_PER_MATERIAL];
};

layout (std430, set = 1, binding = 0) readonly buffer EntitySceneDataBuffer
{
	EntitySceneData entities[];
} entity_data;

layout (std430, set = 1, binding = 1) readonly buffer MaterialDataBuffer
{
	MaterialData materials[];
} material_data;

layout (std430, set = 1, binding = 2) buffer CompactedObjectInstanceBuffer
{
	CompactedObjectInstance instances[];
} compacted_object_instance_buffer;

void main()
{
	out_frag_color = vec4(in_color, 1.0f);
}