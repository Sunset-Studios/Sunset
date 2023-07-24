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

struct DebugPrimitiveData
{
	mat4 transform;
	vec4 color;
};

layout (std430, set = 1, binding = 0) readonly buffer DebugPrimitiveBuffer
{
	DebugPrimitiveData primitives[];
} debug_primitive_data;

void main()
{
	mat4 model_matrix = debug_primitive_data.primitives[gl_InstanceIndex].transform;
	vec4 color = debug_primitive_data.primitives[gl_InstanceIndex].color;

	gl_Position = camera_data.view_proj * model_matrix * vec4(in_position, 1.0f);
	out_color = color.rgb;
	out_tex_coord = in_tex_coord;
}