#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec3 in_position;
layout (location = 4) flat in uint in_instance_index;
layout (location = 5) in mat3 in_tbn_matrix;

layout (location = 0) out vec4 out_frag_color;
layout (location = 1) out float out_specular;
layout (location = 2) out vec4 out_normal;
layout (location = 3) out vec4 out_position;

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

layout (push_constant) uniform constants
{
	vec4 user_data;
} push_constant_uniforms;

void main()
{
	EntitySceneData entity = entity_data.entities[in_instance_index];
	MaterialData material = material_data.materials[entity.material_index];

	const int albedo_tex_index = material.textures[0];
	const int normal_tex_index = material.textures[1];
	const int roughness_tex_index = material.textures[2];

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

	out_frag_color = vec4(albedo, 1.0f);
	out_specular = shininess;
	out_normal = vec4(normal, 0.0f);
	out_position = vec4(in_position, 1.0f);
}
