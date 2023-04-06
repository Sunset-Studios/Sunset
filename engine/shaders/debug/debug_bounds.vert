#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_color;
layout (location = 3) in vec2 in_tex_coord;
layout (location = 4) in vec3 in_tangent;
layout (location = 5) in vec3 in_bitangent;

layout (location = 0) out vec3 out_color;
layout (location = 1) out vec2 out_tex_coord;
layout (location = 2) out uint out_instance_index;

struct EntitySceneData
{
	mat4 transform;
	vec4 bounds_pos_radius;
	vec4 bounds_extent;
	int material_index;
};

layout (std430, set = 1, binding = 0) readonly buffer EntitySceneDataBuffer
{
	EntitySceneData entities[];
} entity_data;

layout (std430, set = 1, binding = 2) buffer CompactedObjectInstanceBuffer
{
	uint ids[];
} compacted_object_instance_buffer;

layout (push_constant) uniform constants
{
	vec4 user_data;
} push_constant_uniforms;

void main()
{
	uint entity_index = compacted_object_instance_buffer.ids[gl_InstanceIndex];
	vec4 pos_radius = entity_data.entities[entity_index].bounds_pos_radius;
	mat4 model_matrix = mat4(
	    vec4(pos_radius.w, 0.0, 0.0, 0.0),
	    vec4(0.0, pos_radius.w, 0.0, 0.0),
	    vec4(0.0, 0.0, pos_radius.w, 0.0),
	    vec4(pos_radius.xyz, 1.0)
	);
	gl_Position = camera_data.view_proj * model_matrix * vec4(in_position, 1.0f);
	out_color = in_color;
	out_tex_coord = in_tex_coord;
	out_instance_index = entity_index;
}