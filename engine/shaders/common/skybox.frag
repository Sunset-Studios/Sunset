#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_frag_position;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	vec3 sample_dir = cube_dir_to_tex_coord_and_layer(in_frag_position);

	vec3 env_color = scene_lighting_data.sky_box == -1
		? vec3(0.0, 0.0, 0.0)
		: texture(textures_2DArray[nonuniformEXT(scene_lighting_data.sky_box)], sample_dir).rgb;

	env_color = env_color / (env_color + vec3(1.0));
	env_color = pow(env_color, vec3(1.0 / 2.2));

	out_frag_color = vec4(env_color, 0.0f);
}