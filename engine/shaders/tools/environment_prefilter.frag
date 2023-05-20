#version 460

#include "common/common.inc"
#include "common/pbr_common.inc"

layout (location = 0) in vec3 in_local_frag_pos;

layout (location = 0) out vec4 out_frag_color;

layout (push_constant) uniform constants
{
	mat4 projection;
	mat4 view;
	int equirect_map_index;
	int layer_index;
	float source_cubemap_resolution;
	float roughness;
} prefilter_cubemap_constants;

const uint sample_count = 1024;

void main()
{
	vec3 normal = normalize(in_local_frag_pos);
	vec3 ref = normal;
	vec3 view = normal;

	vec3 prefiltered_color = vec3(0.0);
	float total_weight = 0.0;

	for (uint i = 0; i < sample_count; ++i)
	{
		vec2 xi = hammersley(i, sample_count);
		vec3 h = importance_sample_ggx(xi, normal, prefilter_cubemap_constants.roughness);
		vec3 l = normalize(2.0 * dot(view, h) * h - view);

		float n_dot_l = max(dot(normal, l), 0.0);
		if (n_dot_l > 0.0)
		{
			float n_dot_h = max(dot(normal, h), 0.0);
			float h_dot_v = max(dot(h, view), 0.0);
			float dggx = d_ggx(n_dot_h, prefilter_cubemap_constants.roughness);
			float pdf = dggx * n_dot_h / (4.0 * h_dot_v) + 0.0001;

			float sa_texel = 4.0 * PI / (6.0 * prefilter_cubemap_constants.source_cubemap_resolution * prefilter_cubemap_constants.source_cubemap_resolution);
			float sa_sample = 1.0 / (float(sample_count) * pdf * 0.0001);

			float mip_level = prefilter_cubemap_constants.roughness == 0.0 ? 0.0 : 0.5 * log2(sa_sample / sa_texel);

			// TODO: use mip_level to textureLod into specific cubemap mip 
			prefiltered_color += textureLod(textures_2DArray[nonuniformEXT(prefilter_cubemap_constants.equirect_map_index)], l, mip_level).rgb * n_dot_l;
			total_weight += n_dot_l;
		}
	}

	prefiltered_color *= (1.0 / float(total_weight));

	out_frag_color = vec4(prefiltered_color, 1.0f);
}