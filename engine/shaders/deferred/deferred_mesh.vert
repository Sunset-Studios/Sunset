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
layout (location = 2) out vec3 out_normal;
layout (location = 3) out vec3 out_position;
layout (location = 4) out uint out_instance_index;
layout (location = 5) out mat3 out_tbn_matrix;

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
	const uint entity_index = compacted_object_instance_buffer.ids[gl_InstanceIndex];
	const mat4 model_matrix = entity_data.entities[entity_index].transform;
	const mat3 transpose_inverse_model_matrix = mat3(transpose(inverse(model_matrix)));
	const vec4 world_position = model_matrix * vec4(in_position, 1.0f);

	vec3 n = normalize((transpose_inverse_model_matrix * in_normal));
	vec3 t = normalize((model_matrix * vec4(in_tangent, 0.0f)).xyz);
	vec3 b = normalize((model_matrix * vec4(in_bitangent, 0.0f)).xyz);

	gl_Position = camera_data.view_proj * world_position;
	out_color = in_color;
	out_tex_coord = in_tex_coord;
	out_normal = transpose_inverse_model_matrix * in_normal;
	out_position = world_position.xyz;
	out_instance_index = entity_index;
	out_tbn_matrix = mat3(t, b, n);
}