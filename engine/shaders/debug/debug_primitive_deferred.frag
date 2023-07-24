#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;

layout (location = 0) out vec4 out_frag_color;

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
	out_frag_color = vec4(in_color, 1.0f);
}