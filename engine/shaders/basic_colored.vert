#version 450

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
} CameraData;

layout (push_constant) uniform constants
{
	mat4 transform_matrix;
	vec4 user_data;
} PushConstantUniforms;

void main()
{
	mat4 transform_matrix = CameraData.view_proj * PushConstantUniforms.transform_matrix;
	gl_Position = transform_matrix * vec4(in_position, 1.0f);
	out_color = in_color;
}