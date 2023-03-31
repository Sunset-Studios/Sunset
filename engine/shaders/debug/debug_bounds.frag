#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) flat in uint in_instance_index;

layout (location = 0) out vec4 out_frag_color;

struct EntitySceneData
{
	mat4 transform;
	vec4 bounds_pos_radius;
	vec4 bounds_extent;
	int material_index;
};

struct MaterialData
{
	int textures[MAX_TEXTURES_PER_MATERIAL];
};

layout (std430, set = 1, binding = 0) readonly buffer EntitySceneDataBuffer
{
	EntitySceneData entities[];
} entity_data;

layout (std140, set = 1, binding = 1) readonly buffer MaterialDataBuffer
{
	MaterialData materials[];
} material_data;

layout (set = 1, binding = 2) buffer CompactedObjectInstanceBuffer
{
	uint ids[];
} compacted_object_instance_buffer;

layout (push_constant) uniform constants
{
	vec4 user_data;
} push_constant_uniforms;

void main()
{
	out_frag_color = vec4(in_color, 1.0f);	
}