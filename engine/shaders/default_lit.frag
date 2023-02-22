#version 460

#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_TEXTURES_PER_MATERIAL 16

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 3) flat in int in_instance_index;

layout (location = 0) out vec4 out_frag_color;

layout (set = 0, binding = 1) uniform SceneLightingData
{
	vec4 fog_color;
	vec4 fog_distance;
	vec4 ambient_color;
	vec4 sunlight_direction;
	vec4 sunlight_color;
} scene_lighting_data;

struct EntitySceneData
{
	mat4 transform;
	uint material_index;
};

struct MaterialData
{
	uint textures[MAX_TEXTURES_PER_MATERIAL];
};

layout (std140, set = 1, binding = 0) readonly buffer EntitySceneDataBuffer
{
	EntitySceneData entities[];
} entity_data;

layout (std140, set = 1, binding = 1) readonly buffer MaterialDataBuffer
{
	MaterialData materials[];
} material_data;

layout (set = 1, binding = 2) uniform sampler2D albedo_textures[];

void main()
{
	EntitySceneData entity = entity_data.entities[in_instance_index];
	MaterialData material = material_data.materials[entity.material_index];
	vec3 color = texture(albedo_textures[material.textures[0]], in_tex_coord).xyz;
	out_frag_color = vec4(color, 1.0f);	
}