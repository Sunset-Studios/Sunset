#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_color;
layout (location = 3) in vec2 in_tex_coord;
layout (location = 4) in vec3 in_tangent;
layout (location = 5) in vec3 in_bitangent;

layout (location = 0) out vec3 out_world_frag_pos;

layout (push_constant) uniform constants
{
	mat4 projection;
	mat4 view;
	int equirect_map_index;
	int layer_index;
} equirect_to_cubemap_constants;

void main()
{
	out_world_frag_pos = in_position;
	gl_Layer = equirect_to_cubemap_constants.layer_index;
	gl_Position = equirect_to_cubemap_constants.projection * equirect_to_cubemap_constants.view * vec4(in_position, 1.0f);
}