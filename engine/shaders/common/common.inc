#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_DESCRIPTOR_BINDINGS 16535
#define MAX_TEXTURES_PER_MATERIAL 16

layout (std140, set = 0, binding = 0) uniform CameraBuffer
{
	mat4 view;
	mat4 proj;
	mat4 view_proj;
	mat4 prev_view_proj;
	mat4 inverse_view_proj;
	vec4 frustum_planes[6];
	vec4 position;
} camera_data;

layout (std140, set = 0, binding = 1) uniform SceneLightingData
{
	vec4 fog_color;
	vec4 fog_distance;
	vec4 ambient_color;
	vec4 sunlight_direction;
	vec4 sunlight_color;
	float num_lights;
	float padding[3];
} scene_lighting_data;

layout (set = 0, binding = 2) uniform writeonly image2D storage_2D[MAX_DESCRIPTOR_BINDINGS];
layout (set = 0, binding = 2) uniform writeonly image3D storage_3D[MAX_DESCRIPTOR_BINDINGS];

layout (set = 0, binding = 3) uniform sampler2D textures_2D[];
layout (set = 0, binding = 3) uniform sampler3D textures_3D[];