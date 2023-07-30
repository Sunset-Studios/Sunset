#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec3 in_position;
layout (location = 4) flat in uint in_instance_index;
layout (location = 5) flat in uint in_material_index;
layout (location = 6) in mat3 in_tbn_matrix;

layout (location = 0) out vec4 out_frag_color;
layout (location = 1) out vec4 out_smra;
layout (location = 2) out vec2 out_cc;
layout (location = 3) out vec4 out_normal;
layout (location = 4) out vec4 out_position;

struct EntitySceneData
{
	mat4 transform;
	vec4 bounds_pos_radius;
	vec4 bounds_extent_and_custom_scale;
};

struct CompactedObjectInstance
{
	uint object_id;
	uint material_id;
};

struct MaterialData
{
	vec3 color;
	float uniform_roughness;
	float uniform_metallic;
	float uniform_reflectance;
	float uniform_clearcoat;
	float uniform_clearcoat_roughness;
	float uniform_emissive;
	float padding[3];
	int textures[MAX_TEXTURES_PER_MATERIAL];
	float tiling_coeffs[MAX_TEXTURES_PER_MATERIAL];
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
	CompactedObjectInstance instances[];
} compacted_object_instance_buffer;

void main()
{
	MaterialData material = material_data.materials[in_material_index];

	const int albedo_tex_index = material.textures[0];
	const int normal_tex_index = material.textures[1];
	const int roughness_tex_index = material.textures[2];
	const int metallic_tex_index = material.textures[3];
	const int ao_tex_index = material.textures[4];
	const int emissive_tex_index = material.textures[5];

	const vec3 albedo = 
		albedo_tex_index == -1
		? material.color
		: texture(textures_2D[nonuniformEXT(albedo_tex_index)], in_tex_coord * material.tiling_coeffs[0]).xyz;

	const vec3 normal =
		normal_tex_index == -1
		? normalize(in_normal)
		: in_tbn_matrix * normalize(texture(textures_2D[nonuniformEXT(normal_tex_index)], in_tex_coord * material.tiling_coeffs[1]).xyz * 2.0f - 1.0f);

	const float roughness =
		roughness_tex_index == -1
		? material.uniform_roughness
		: texture(textures_2D[nonuniformEXT(roughness_tex_index)], in_tex_coord * material.tiling_coeffs[2]).r;

	const float metallic =
		metallic_tex_index == -1
		? material.uniform_metallic
		: texture(textures_2D[nonuniformEXT(metallic_tex_index)], in_tex_coord * material.tiling_coeffs[3]).r + 0.1f;

	const float ao =
		ao_tex_index == -1
		? 1.0f
		: texture(textures_2D[nonuniformEXT(ao_tex_index)], in_tex_coord * material.tiling_coeffs[4]).r;

	const float emissive =
		emissive_tex_index == -1
		? material.uniform_emissive
		: texture(textures_2D[nonuniformEXT(emissive_tex_index)], in_tex_coord * material.tiling_coeffs[5]).r * material.uniform_emissive;

	const float specular = material.uniform_reflectance > 0.0f ? material.uniform_reflectance : pow(2.0f, 10.0f * (1.0f - roughness));

	out_frag_color = vec4(albedo, emissive);
	out_smra.r = specular;
	out_smra.g = metallic;
	out_smra.b = roughness;
	out_smra.a = ao;
	out_cc.r = material.uniform_clearcoat;
	out_cc.g = material.uniform_clearcoat_roughness;
	out_normal = vec4(normal, 1.0);
	out_position = vec4(in_position, in_instance_index);
}
