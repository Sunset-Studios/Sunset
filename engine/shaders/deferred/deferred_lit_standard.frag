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
	vec4 bounds_extent;
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
	int sky_texture;
	int ssao_texture;
	int shadow_texture;
} lighting_pass_constants;

void main()
{
	const vec3 ambient = vec3(0.1f, 0.1f, 0.1f);

	vec4 tex_albedo = 
		lighting_pass_constants.albedo_texure == -1
		? vec4(0.0, 0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.albedo_texure)], in_tex_coord);
	const vec3 albedo = tex_albedo.rgb;
	const float emissive = tex_albedo.a;

	vec4 tex_normal =
		lighting_pass_constants.normal_texure == -1
		? vec4(0.0, 0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.normal_texure)], in_tex_coord);
	const vec3 normal = tex_normal.xyz;
	const float normal_length = length(normal);
	const vec3 normalized_normal = normal / normal_length;

	vec4 tex_smra = 
		lighting_pass_constants.smra_texure == -1
		? vec4(0.0, 0.0, 0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.smra_texure)], in_tex_coord);
	const float reflectance = tex_smra.r * 0.0009765625 /* 1.0f / 1024 */;
	const float metallic = tex_smra.g;
	const float roughness = tex_smra.b;
	const float ao = tex_smra.a;

	vec2 tex_cc = 
		lighting_pass_constants.cc_texture == -1
		? vec2(0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.cc_texture)], in_tex_coord).rg;
	const float clearcoat = tex_cc.r;
	const float clearcoat_roughness = tex_cc.g;

	vec4 tex_position = 
		lighting_pass_constants.position_texure == -1
		? vec4(0.0, 0.0, 0.0, 1.0)
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.position_texure)], in_tex_coord);
	const vec3 position = tex_position.xyz;

	vec4 tex_sky = 
		lighting_pass_constants.sky_texture == -1
		? tex_albedo
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.sky_texture)], in_tex_coord);

	float tex_ssao = 
		lighting_pass_constants.ssao_texture == -1
		? 1.0
		: texture(textures_2D[nonuniformEXT(lighting_pass_constants.ssao_texture)], in_tex_coord).r;

	vec4 tex_irradiance =
		scene_lighting_data.irradiance_map == -1
		? vec4(0.0, 0.0, 0.0, 1.0)
		: texture(textures_2DArray[nonuniformEXT(scene_lighting_data.irradiance_map)], cube_dir_to_tex_coord_and_layer(normalized_normal));
	const vec3 irradiance = tex_irradiance.rgb; 

	const vec3 view_dir = normalize(camera_data.position.xyz - position);
	const vec3 reflection = reflect(-view_dir, normalized_normal);

	vec4 tex_prefilter =
		scene_lighting_data.prefilter_map == -1
		? vec4(0.0, 0.0, 0.0, 1.0)
		: textureLod(textures_2DArray[nonuniformEXT(scene_lighting_data.prefilter_map)], cube_dir_to_tex_coord_and_layer(reflection), roughness * 4.0);
	const vec3 prefiltered_color = tex_prefilter.rgb;

	vec2 tex_env_brdf =
		scene_lighting_data.brdf_lut == -1
		? vec2(0.0, 0.0)
		: texture(textures_2D[nonuniformEXT(scene_lighting_data.brdf_lut)], vec2(max(dot(normalized_normal, view_dir), 0.0), roughness)).rg;

	const float unlit = float(normal_length <= 0.0);

	vec3 color = unlit * tex_sky.rgb;

	const uint num_light_iterations = uint((1 - unlit) * scene_lighting_data.num_lights);
	for (uint i = 0; i < num_light_iterations; ++i)
	{
		LightData light = light_data.lights[i];
		const vec3 light_position = entity_data.entities[light.entity].transform[3].xyz;
		color += calculate_brdf(
			light_data.lights[i],
			light_position,
			normalized_normal,
			view_dir,
			position,
			albedo,
			roughness,
			metallic,
			reflectance,
			clearcoat,
			clearcoat_roughness,
			min(ao, tex_ssao),
			irradiance,
			prefiltered_color,
			tex_env_brdf,
			lighting_pass_constants.shadow_texture);
	}

	out_frag_color = vec4(color + (albedo * emissive), 1.0f);
}
