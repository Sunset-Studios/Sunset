#version 460

layout (location = 0) in vec3 in_color;

layout (location = 0) out vec4 out_frag_color;

layout (set = 0, binding = 1) uniform SceneLightingData
{
	vec4 fog_color;
	vec4 fog_distance;
	vec4 ambient_color;
	vec4 sunlight_direction;
	vec4 sunlight_color;
} scene_lighting_data;

void main()
{
	out_frag_color = vec4(in_color + scene_lighting_data.ambient_color.xyz, 1.0f);	
}