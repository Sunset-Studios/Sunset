#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_local_frag_pos;

layout (location = 0) out vec4 out_frag_color;

layout (push_constant) uniform constants
{
	mat4 projection;
	mat4 view;
	int equirect_map_index;
	int layer_index;
} equirect_to_cubemap_constants;

const vec2 inv_atan = vec2(0.1591, 0.3183);

vec2 sample_spherical_map(vec3 dir)
{
	vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y));
	uv = (uv * inv_atan) + 0.5;
	return uv;
}

void main()
{
	vec2 uv = sample_spherical_map(normalize(in_local_frag_pos));
	vec3 color = texture(textures_2D[nonuniformEXT(equirect_to_cubemap_constants.equirect_map_index)], uv).rgb;
	color = min(color, vec3(500.0));
	out_frag_color = vec4(color, 1.0f);
}