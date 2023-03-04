#version 460

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_color;
layout (location = 3) in vec2 in_tex_coord;

layout (location = 0) out vec3 out_color;
layout (location = 1) out vec2 out_tex_coord;
layout (location = 2) out int out_instance_index;

// TODO: Put this global data in a shader include
layout (set = 0, binding = 0) uniform CameraBuffer
{
	mat4 view;
	mat4 proj;
	mat4 view_proj;
	mat4 inverse_view_proj;
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

struct EntitySceneData
{
	mat4 transform;
	vec4 bounds_pos_radius;
	vec4 bounds_extent;
	uint material_index;
};

layout (std140, set = 1, binding = 0) readonly buffer EntitySceneDataBuffer
{
	EntitySceneData entities[];
} entity_data;

layout (push_constant) uniform constants
{
	vec4 user_data;
} push_constant_uniforms;

void main()
{
	mat4 model_matrix = entity_data.entities[gl_BaseInstance].transform;
	mat4 transform_matrix = camera_data.view_proj * model_matrix;
	gl_Position = transform_matrix * vec4(in_position, 1.0f);
	out_color = in_color;
	out_tex_coord = in_tex_coord;
	out_instance_index = gl_BaseInstance;
}