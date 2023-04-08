#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec3 in_position;
layout (location = 4) flat in uint in_instance_index;
layout (location = 5) in mat3 in_tbn_matrix;

layout (location = 0) out vec4 out_frag_color;
layout (location = 1) out vec4 out_frag_bright_color;

struct EntitySceneData
{
	mat4 transform;
	vec4 bounds_pos_radius;
	vec4 bounds_extent;
	int material_index;
};

struct MaterialData
{
	int textures[MAX_TEXTURES_PER_MATERIAL];
	float tiling_coeffs[MAX_TEXTURES_PER_MATERIAL];
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

layout (std430, set = 1, binding = 1) readonly buffer MaterialDataBuffer
{
	MaterialData materials[];
} material_data;

layout (std430, set = 1, binding = 2) buffer CompactedObjectInstanceBuffer
{
	uint ids[];
} compacted_object_instance_buffer;

layout (std430, set = 1, binding = 3) buffer LightDataBuffer
{
	LightData lights[];
} light_data;

layout (push_constant) uniform constants
{
	vec4 user_data;
} push_constant_uniforms;

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
	EntitySceneData entity = entity_data.entities[in_instance_index];
	MaterialData material = material_data.materials[entity.material_index];

	const int albedo_tex_index = material.textures[0];
	const int normal_tex_index = material.textures[1];
	const int roughness_tex_index = material.textures[2];

	const vec3 ambient = vec3(0.1f, 0.1f, 0.1f);

	const vec3 albedo = 
		albedo_tex_index == -1
		? vec3(0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(albedo_tex_index)], in_tex_coord * material.tiling_coeffs[0]).xyz;

	vec3 normal =
		normal_tex_index == -1
		? in_normal
		: in_tbn_matrix * normalize(texture(textures_2D[nonuniformEXT(normal_tex_index)], in_tex_coord * material.tiling_coeffs[1]).xyz * 2.0f - 1.0f);

	const float roughness =
		roughness_tex_index == -1
		? 0.0f
		: texture(textures_2D[nonuniformEXT(roughness_tex_index)], in_tex_coord * material.tiling_coeffs[2]).r;

	const float shininess = pow(2.0f, 10.0f * (1.0f - roughness));

	const vec3 view_dir = normalize(in_position - camera_data.position.xyz);

	vec3 color = vec3(0.0f, 0.0f, 0.0f);	
	for (int i = 0; i < scene_lighting_data.num_lights; ++i)
	{
		LightData light = light_data.lights[i];
		const vec3 light_position = entity_data.entities[light.entity].transform[3].xyz;
		color += calculate_blinn_phong(light_data.lights[i], light_position, normal, view_dir, in_position, albedo, shininess, ambient);
	}

	out_frag_color = vec4(color, 1.0f);

	const float brightness = dot(out_frag_color.rgb, vec3(0.2126f, 0.7152f, 0.0722f));
	out_frag_bright_color = vec4(int(brightness > 1.0f) * out_frag_color.rgb, 1.0f);
}
