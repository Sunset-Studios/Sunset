#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) flat in int in_instance_index;

layout (location = 0) out vec4 out_frag_color;

layout (push_constant) uniform constants
{
	int scene_color_index;
} fullscreen_push_constants;

void main()
{
	vec3 color = vec3(0.0f, 0.0f, 0.0f);

	if (fullscreen_push_constants.scene_color_index > -1)
	{
		color = texture(textures_2D[nonuniformEXT(fullscreen_push_constants.scene_color_index)], in_tex_coord).xyz;
	}

	out_frag_color = vec4(color, 1.0f);
}