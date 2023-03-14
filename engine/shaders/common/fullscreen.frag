#version 460

#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_TEXTURES_PER_MATERIAL 16

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) flat in int in_instance_index;

layout (location = 0) out vec4 out_frag_color;

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
layout (set = 0, binding = 2) uniform sampler2D textures_2D[];
layout (set = 0, binding = 2) uniform sampler3D textures_3D[];

layout (push_constant) uniform constants
{
	int scene_color_index;
} fullscreen_push_constants;

void main()
{
	vec3 color = vec3(0.0f, 0.0f, 0.0f);

	if (fullscreen_push_constants.scene_color_index > -1)
	{
		color = texture(textures_2D[nonuniformEXT(fullscreen_push_constants.scene_color_index)], in_tex_coord).xyz;
	}

	out_frag_color = vec4(color, 1.0f);
}