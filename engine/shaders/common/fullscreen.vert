#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_color;
layout (location = 3) in vec2 in_tex_coord;

layout (location = 0) out vec3 out_color;
layout (location = 1) out vec2 out_tex_coord;
layout (location = 2) out int out_instance_index;

void main()
{
	gl_Position = vec4(in_position, 1.0f);
	out_color = in_color;
	out_tex_coord = in_tex_coord;
	out_instance_index = gl_InstanceIndex;
}