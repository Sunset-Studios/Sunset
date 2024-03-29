#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_color;
layout (location = 3) in vec2 in_tex_coord;
layout (location = 4) in vec3 in_tangent;
layout (location = 5) in vec3 in_bitangent;

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

layout (std430, set = 1, binding = 0) readonly buffer EntitySceneDataBuffer
{
	EntitySceneData entities[];
} entity_data;

layout (std430, set = 1, binding = 2) buffer CompactedObjectInstanceBuffer
{
	CompactedObjectInstance instances[];
} compacted_object_instance_buffer;

void main()
{
	const uint entity_index = compacted_object_instance_buffer.instances[gl_InstanceIndex].object_id;
	const mat4 model_matrix = entity_data.entities[entity_index].transform;
	const vec4 world_position = model_matrix * vec4(in_position, 1.0f);

	gl_Position = camera_data.view_proj * world_position;
}