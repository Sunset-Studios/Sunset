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

	// Adjust the projection matrix to remove any jitter, which is used for temporal anti-aliasing.
	mat4 unjittered_proj = camera_data.proj;
	unjittered_proj[2][0] -= camera_data.jitter.z;
	unjittered_proj[2][1] -= camera_data.jitter.w;

	vec4 clip_pos = unjittered_proj * rotation_view * vec4(out_frag_position, 1.0);

	gl_Position = clip_pos.xyww;
}