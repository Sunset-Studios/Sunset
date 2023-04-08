#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) flat in int in_instance_index;

layout (location = 0) out vec4 out_frag_color;

layout (push_constant) uniform constants
{
	int scene_color_index;
	int bloom_brightness_index;
	float exposure;
	float bloom_intensity;
} bloom_resolve_constants;

const float gamma = 2.2f;

void main()
{
	vec3 color = vec3(0.0f, 0.0f, 0.0f);

	if (bloom_resolve_constants.scene_color_index > -1)
	{
		color = texture(textures_2D[nonuniformEXT(bloom_resolve_constants.scene_color_index)], in_tex_coord).rgb;
	}

	if (bloom_resolve_constants.bloom_brightness_index > -1)
	{
		vec3 bloom_color = texture(textures_2D[nonuniformEXT(bloom_resolve_constants.bloom_brightness_index)], in_tex_coord).rgb;
		color = mix(color, bloom_color, bloom_resolve_constants.bloom_intensity);
	}

	color = vec3(1.0f) - exp(-color * bloom_resolve_constants.exposure);
	color = pow(color, vec3(1.0f / gamma));

	out_frag_color = vec4(color, 1.0f);
}