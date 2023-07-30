#version 460

#include "common/common.inc"
#include "common/lighting_common.inc"

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) flat in int in_instance_index;

layout (location = 0) out vec4 out_frag_color;

struct EntitySceneData
{
	mat4 transform;
	vec4 bounds_pos_radius;
	vec4 bounds_extent_and_custom_scale;
};

layout (std430, set = 1, binding = 0) readonly buffer EntitySceneDataBuffer
{
	EntitySceneData entities[];
} entity_data;

layout (std430, set = 1, binding = 1) buffer LightDataBuffer
{
	LightData lights[];
} light_data;

layout (push_constant) uniform constants
{
	int albedo_texure;
	int smra_texure;
	int cc_texture;
	int normal_texure;
	int position_texure;
} lighting_pass_constants;

void main()
{
	const vec3 albedo = 
		lighting_pass_constants.albedo_texure == -1
		? vec3(0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.albedo_texure)], in_tex_coord).rgb;

	const vec3 normal =
		lighting_pass_constants.normal_texure == -1
		? vec3(0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.normal_texure)], in_tex_coord).xyz;
	const float normal_length = length(normal);

	const float specular =
		lighting_pass_constants.smra_texure == -1
		? 0.0f
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.smra_texure)], in_tex_coord).r;

	const vec3 position =
		lighting_pass_constants.position_texure == -1
		? vec3(0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.position_texure)], in_tex_coord).xyz;

	const vec3 view_dir = normalize(position - camera_data.position.xyz);

	const float unlit = float(normal_length <= 0.0);

	vec3 color = unlit * albedo;

	const uint num_light_iterations = uint((1 - unlit) * scene_lighting_data.num_lights);
	for (uint i = 0; i < num_light_iterations; ++i)
	{
		LightData light = light_data.lights[i];
		const vec3 light_position = entity_data.entities[light.entity].transform[3].xyz;
		color += calculate_blinn_phong(light_data.lights[i], light_position, normal / normal_length, view_dir, position, albedo, specular, vec3(0.1f));
	}

	out_frag_color = vec4(color, 1.0f);
}
