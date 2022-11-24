#version 450

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_color;

layout (location = 0) out vec3 out_color;

layout (push_constant) uniform constants
{
	mat4 transform_matrix;
	vec4 user_data;
} PushConstantUniforms;

void main()
{
	gl_Position = PushConstantUniforms.transform_matrix * vec4(in_position, 1.0f);
	out_color = in_color;
}