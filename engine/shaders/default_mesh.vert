#version 460

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_color;

layout (location = 0) out vec3 out_color;

layout (set = 0, binding = 0) uniform CameraBuffer
{
	mat4 view;
	mat4 proj;
	mat4 view_proj;
	mat4 inverse_view_proj;
} camera_data;

layout (std140, set = 1, binding = 0) readonly buffer EntityTransforms
{
	mat4 transforms[];
} entity_transforms;

layout (push_constant) uniform constants
{
	vec4 user_data;
} push_constant_uniforms;

void main()
{
	mat4 model_matrix = entity_transforms.transforms[gl_BaseInstance];
	mat4 transform_matrix = camera_data.view_proj * model_matrix;
	gl_Position = transform_matrix * vec4(in_position, 1.0f);
	out_color = in_color;
}