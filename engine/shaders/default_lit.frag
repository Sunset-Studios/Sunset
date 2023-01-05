#version 460

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;

layout (location = 0) out vec4 out_frag_color;

layout (set = 0, binding = 1) uniform SceneLightingData
{
	vec4 fog_color;
	vec4 fog_distance;
	vec4 ambient_color;
	vec4 sunlight_direction;
	vec4 sunlight_color;
} scene_lighting_data;

layout (set = 2, binding = 0) uniform sampler2D albedo_texture;

void main()
{
	vec3 color = texture(albedo_texture, in_tex_coord).xyz;
	out_frag_color = vec4(color, 1.0f);	
}