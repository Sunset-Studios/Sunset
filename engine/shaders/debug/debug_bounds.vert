#version 460

#define MAX_DESCRIPTOR_BINDINGS 16535

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_color;
layout (location = 3) in vec2 in_tex_coord;

layout (location = 0) out vec3 out_color;
layout (location = 1) out vec2 out_tex_coord;
layout (location = 2) out uint out_instance_index;

// TODO: Put this global data in a shader include
layout (set = 0, binding = 0) uniform CameraBuffer
{
	mat4 view;
	mat4 proj;
	mat4 view_proj;
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

layout (std430, set = 1, binding = 0) readonly buffer EntitySceneDataBuffer
{
	EntitySceneData entities[];
} entity_data;

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
	uint entity_index = compacted_object_instance_buffer.ids[gl_InstanceIndex];
	vec4 pos_radius = entity_data.entities[entity_index].bounds_pos_radius;
	mat4 model_matrix = mat4(
	    vec4(pos_radius.w, 0.0, 0.0, 0.0),
	    vec4(0.0, pos_radius.w, 0.0, 0.0),
	    vec4(0.0, 0.0, pos_radius.w, 0.0),
	    vec4(pos_radius.xyz, 1.0)
	);
	mat4 transform_matrix = camera_data.view_proj * model_matrix;
	gl_Position = transform_matrix * vec4(in_position, 1.0f);
	out_color = in_color;
	out_tex_coord = in_tex_coord;
	out_instance_index = entity_index;
}