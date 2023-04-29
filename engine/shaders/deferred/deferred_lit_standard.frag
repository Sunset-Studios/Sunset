#version 460

#include "common/common.inc"
#include "common/pbr_common.inc"
#include "common/lighting_common.inc"

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
	const vec3 ambient = vec3(0.1f, 0.1f, 0.1f);

	vec4 tex_albedo = 
		lighting_pass_constants.albedo_texure == -1
		? vec4(0.0, 0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.albedo_texure)], in_tex_coord);

	vec4 tex_normal =
		lighting_pass_constants.normal_texure == -1
		? vec4(0.0, 0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.normal_texure)], in_tex_coord);

	vec4 tex_smra = 
		lighting_pass_constants.smra_texure == -1
		? vec4(0.0, 0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.smra_texure)], in_tex_coord);

	vec2 tex_cc = 
		lighting_pass_constants.cc_texture == -1
		? vec2(0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.cc_texture)], in_tex_coord).rg;

	vec4 tex_position = 
		lighting_pass_constants.position_texure == -1
		? vec4(0.0, 0.0, 0.0, 1.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.position_texure)], in_tex_coord);

	const vec3 albedo = tex_albedo.rgb;
	const vec3 normal = tex_normal.xyz;
	const float normal_length = length(normal);
	const float reflectance = tex_smra.r * 0.0009765625 /* 1.0f / 1024 */;
	const float metallic = tex_smra.g;
	const float roughness = tex_smra.b;
	const float ao = tex_smra.a;
	const float clearcoat = tex_cc.r;
	const float clearcoat_roughness = tex_cc.g;
	const vec3 position = tex_position.xyz;

	const vec3 view_dir = normalize(position - camera_data.position.xyz);

	const float unlit = float(normal_length <= 0.0);

	vec3 color = unlit * albedo * 0.1f;

	const uint num_light_iterations = uint((1 - unlit) * scene_lighting_data.num_lights);
	for (uint i = 0; i < num_light_iterations; ++i)
	{
		LightData light = light_data.lights[i];
		const vec3 light_position = entity_data.entities[light.entity].transform[3].xyz;
		color += calculate_brdf(light_data.lights[i], light_position, normal / normal_length, view_dir, position, albedo, roughness, metallic, reflectance, clearcoat, clearcoat_roughness, ao);
	}

	out_frag_color = vec4(color, 1.0f);
}
