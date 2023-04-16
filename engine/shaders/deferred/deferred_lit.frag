#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) flat in int in_instance_index;

layout (location = 0) out vec4 out_frag_color;

struct EntitySceneData
{
	mat4 transform;
	vec4 bounds_pos_radius;
	vec4 bounds_extent;
	int material_index;
};

struct LightData
{
	vec4 color;
	vec4 direction;
	float radius;
	float outer_angle;
	vec2 area_size;
	uint type;
	int entity;
	float intensity;
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
	int specular_texure;
	int normal_texure;
	int position_texure;
} lighting_pass_constants;

vec3 calculate_blinn_phong(LightData light, vec3 position, vec3 normal, vec3 view_dir, vec3 fragment_pos, vec3 albedo, float shininess, vec3 ambient)
{
	vec3 light_dir;
	float attenuation = 1.0f;

	// Ambient
	vec3 ambient_color = albedo * ambient;

	if (light.type == 0) // Directional
	{
		light_dir = normalize(-position);
	}
	else if (light.type == 1) // Point
	{
		const vec3 light_to_frag = fragment_pos - position;
		light_dir = normalize(light_to_frag);
		const float distance = length(light_to_frag);
		const float falloff = 1.0f - smoothstep(0.0f, light.radius, distance);
		attenuation = falloff / (1.0f + 0.09f * distance + 0.032f * distance * distance);
	}
	else if (light.type == 2) // Spot
	{
		const vec3 light_to_frag = fragment_pos - position;
		light_dir = normalize(light_to_frag);
		const float distance = length(light_to_frag);
		const float spot_effect = dot(light_dir, light.direction.xyz);
		const float falloff = pow(spot_effect, 180.0f / max(1.0f, light.outer_angle));
		attenuation = falloff / (1.0f + 0.09f * distance + 0.032f * distance * distance);
	}

	attenuation = clamp(attenuation, 0.0f, 255.0f);

	// Diffuse
	float n_dot_l = max(dot(normal, light_dir), 0.0f);
	vec3 diffuse_color = light.color.rgb * albedo * n_dot_l;

	// Specular
	vec3 halfway = normalize(light_dir + view_dir);
	float n_dot_h = max(dot(normal, halfway), 0.0f);
	float specular = pow(n_dot_h, shininess);
	vec3 specular_color = light.color.rgb * specular;

	// Attenuation
	vec3 final_color = ambient_color + (diffuse_color + specular_color) * light.intensity * attenuation;

	return final_color;
}

void main()
{
	const vec3 ambient = vec3(0.1f, 0.1f, 0.1f);

	const vec3 albedo = 
		lighting_pass_constants.albedo_texure == -1
		? vec3(0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.albedo_texure)], in_tex_coord).xyz;

	const vec3 normal =
		lighting_pass_constants.normal_texure == -1
		? vec3(0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.normal_texure)], in_tex_coord).xyz;
	const float normal_length = length(normal);

	const float shininess =
		lighting_pass_constants.specular_texure == -1
		? 0.0f
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.specular_texure)], in_tex_coord).r;

	const vec3 position =
		lighting_pass_constants.position_texure == -1
		? vec3(0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.position_texure)], in_tex_coord).xyz;

	const vec3 view_dir = normalize(position - camera_data.position.xyz);
	const uint unlit = uint(normal_length <= 0.0);

	vec3 color = float(unlit) * albedo;

	const uint num_light_iterations = uint((1 - unlit) * scene_lighting_data.num_lights);
	for (uint i = 0; i < num_light_iterations; ++i)
	{
		LightData light = light_data.lights[i];
		const vec3 light_position = entity_data.entities[light.entity].transform[3].xyz;
		color += calculate_blinn_phong(light_data.lights[i], light_position, normal / normal_length, view_dir, position, albedo, shininess, ambient);
	}

	out_frag_color = vec4(color, 1.0f);
}
