#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) flat in int in_instance_index;

layout (location = 0) out vec4 out_frag_color;

layout (push_constant) uniform constants
{
	int smra_texure;
	int ssr_texture;
	int ssr_blurred_texture;
	int scene_color_texture;
	float ssr_strength;
} ssr_resolve_constants;

void main()
{
	vec4 tex_smra = 
		ssr_resolve_constants.smra_texure == -1
		? vec4(0.0, 0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(ssr_resolve_constants.smra_texure)], in_tex_coord);
	const float reflectance = tex_smra.r * 0.0009765625 /* 1.0f / 1024 */;
	const float roughness = tex_smra.b;

	vec3 tex_ssr = 
		ssr_resolve_constants.ssr_texture == -1
		? vec3(0.0, 0.0, 0.0) 
		: texture(textures_2D[nonuniformEXT(ssr_resolve_constants.ssr_texture)], in_tex_coord).rgb;
	vec3 tex_ssr_blurred = 
		ssr_resolve_constants.ssr_blurred_texture == -1
		? vec3(0.0, 0.0, 0.0) 
		: texture(textures_2D[nonuniformEXT(ssr_resolve_constants.ssr_blurred_texture)], in_tex_coord).rgb;
	vec3 ssr_color = mix(tex_ssr, tex_ssr_blurred, roughness);

	vec4 tex_scene_color = 
		ssr_resolve_constants.scene_color_texture == -1
		? vec4(0.0, 0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(ssr_resolve_constants.scene_color_texture)], in_tex_coord);

	out_frag_color = vec4(tex_scene_color.rgb + (ssr_color.rgb * ssr_resolve_constants.ssr_strength * tex_scene_color.rgb * reflectance), 1.0f);
}
