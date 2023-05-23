#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_color;
layout (location = 3) in vec2 in_tex_coord;
layout (location = 4) in vec3 in_tangent;
layout (location = 5) in vec3 in_bitangent;

layout (location = 0) out vec3 out_frag_position;

void main()
{
	out_frag_position = in_position;

	mat4 rotation_view = mat4(mat3(camera_data.view));
	vec4 clip_pos = camera_data.proj * rotation_view * vec4(out_frag_position, 1.0);

	gl_Position = clip_pos.xyww;
}