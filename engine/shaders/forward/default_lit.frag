#version 460

#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_DESCRIPTOR_BINDINGS 16535
#define MAX_TEXTURES_PER_MATERIAL 16

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) flat in uint in_instance_index;

layout (location = 0) out vec4 out_frag_color;

// TODO: Put this global data in a shader include
layout (set = 0, binding = 0) uniform CameraBuffer
{
	mat4 view;
	mat4 proj;
	mat4 view_proj;
	mat4 prev_view_proj;
	mat4 inverse_view_proj;
	vec4 frustum_planes[6];
} camera_data;

// TODO: Put this global data in a shader include
layout (set = 0, binding = 1) uniform SceneLightingData
{
	vec4 fog_color;
	vec4 fog_distance;
	vec4 ambient_color;
	vec4 sunlight_direction;
	vec4 sunlight_color;
} scene_lighting_data;

// TODO: Put this global data in a shader include
layout (set = 0, binding = 2) uniform writeonly image2D storage_2D[MAX_DESCRIPTOR_BINDINGS];
layout (set = 0, binding = 2) uniform writeonly image3D storage_3D[MAX_DESCRIPTOR_BINDINGS];

// TODO: Put this global data in a shader include
layout (set = 0, binding = 3) uniform sampler2D textures_2D[];
layout (set = 0, binding = 3) uniform sampler3D textures_3D[];

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
};

layout (std430, set = 1, binding = 0) readonly buffer EntitySceneDataBuffer
{
	EntitySceneData entities[];
} entity_data;

layout (std140, set = 1, binding = 1) readonly buffer MaterialDataBuffer
{
	MaterialData materials[];
} material_data;

layout (set = 1, binding = 2) buffer CompactedObjectInstanceBuffer
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
	vec3 color = vec3(0.0f, 0.0f, 0.0f);
	if (material.textures[0] > -1)
	{
		color = texture(textures_2D[nonuniformEXT(material.textures[0])], in_tex_coord).xyz;
	}
	out_frag_color = vec4(color, 1.0f);	
}