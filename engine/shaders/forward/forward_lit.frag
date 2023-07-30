#version 460

#include "common/common.inc"
#include "common/lighting_common.inc"

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec3 in_position;
layout (location = 4) flat in uint in_instance_index;
layout (location = 5) flat in uint in_material_index;
layout (location = 6) in mat3 in_tbn_matrix;

layout (location = 0) out vec4 out_frag_color;

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

layout (std430, set = 1, binding = 3) buffer LightDataBuffer
{
	LightData lights[];
} light_data;

void main()
{
	EntitySceneData entity = entity_data.entities[in_instance_index];
	MaterialData material = material_data.materials[in_material_index];

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
}
