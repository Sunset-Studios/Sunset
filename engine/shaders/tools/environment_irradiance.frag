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

void main()
{
	vec3 normal = normalize(in_local_frag_pos);
	vec3 irradiance = vec3(0.0);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, normal));
	up = normalize(cross(normal, right));

	// Simple discrete uniform distribution along the normal hemisphere
	float sample_delta = 0.025;
	float num_samples = 0.0;
	for (float phi = 0.0; phi < 2.0 * PI; phi += sample_delta)
	{
		for(float theta = 0.0; theta < 0.5 * PI; theta += sample_delta)
		{
			vec3 tangent_sample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 sample_vec = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * normal;
			irradiance += texture(textures_2DArray[nonuniformEXT(equirect_to_cubemap_constants.equirect_map_index)], sample_vec).rgb * cos(theta) * sin(theta);
			++num_samples;
		}
	}
	irradiance = PI * irradiance * (1.0 / num_samples);

	out_frag_color = vec4(irradiance, 1.0f);
}