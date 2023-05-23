#version 460

#include "common/common.inc"
#include "common/pbr_common.inc"

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) flat in int in_instance_index;

layout (location = 0) out vec4 out_frag_color;

const uint sample_count = 2048;

float v_schlick_ggx_ibl(float n_dot_v, float roughness)
{
	float a = roughness;
	float k = (a * a) * 0.5;
	float nom = n_dot_v;
	float denom = n_dot_v * (1.0 - k) + k;
	return nom / denom;
}

float v_smith_ggx_ibl(float n_dot_v, float n_dot_l, float roughness)
{
	return v_schlick_ggx_ibl(n_dot_v, roughness) * v_schlick_ggx_ibl(n_dot_l, roughness);
}

void main()
{
	float n_dot_v = in_tex_coord.x;
	float roughness = in_tex_coord.y;

	vec3 v = vec3(sqrt(1.0 - n_dot_v * n_dot_v), 0.0, n_dot_v);

	float a = 0.0;
	float b = 0.0;

	vec3 normal = vec3(0.0, 0.0, 1.0);

	for (uint i = 0; i < sample_count; ++i)
	{
		vec2 xi = hammersley(i, sample_count);
		vec3 h = importance_sample_ggx(xi, normal, roughness);
		vec3 l = normalize(2.0 * dot(v, h) * h - v);

		float n_dot_l = max(l.z, 0.0);
		float n_dot_h = max(h.z, 0.0);
		float v_dot_h = max(dot(v, h), 0.0);

		if (n_dot_l > 0.0)
		{
			float g = v_smith_ggx_ibl(n_dot_v, n_dot_l, roughness);
			float g_vis = (g * v_dot_h) / (n_dot_h * n_dot_v);
			float fc = pow(1.0 - v_dot_h, 5.0);

			a += (1.0 - fc) * g_vis;
			b += fc * g_vis;
		}
	}	

	a /= float(sample_count);
	b /= float(sample_count);

	out_frag_color = vec4(a, b, 0.0, 1.0f);
}