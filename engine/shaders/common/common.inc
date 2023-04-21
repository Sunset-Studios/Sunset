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
	vec4 jitter;
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

float cubic_filter(in float x, in float b, in float c)
{
	float y = 0.0f;
	float x2 = x * x;
	float x3 = x2 * x;
	if (x < 1)
	{
		y = (12.0f - 9.0f * b - 6.0f * c) * x3 + (-18.0f + 12.0f * b + 6.0f * c) * x2 + (6.0f - 2.0f * b);
	}
	else
	{
		y = (-b - 6.0f * c) * x3 + (6.0f * b + 30.0f * c) * x2 + (-12.0f * b - 48.0f * c) * x + (8.0f * b + 24.0f * c);
	}
	return y / 6.0f;
}

float catmull_rom_filter(float value)
{
	return cubic_filter(value, 0.0f, 0.5f);
}

float luminance(vec3 color)
{
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

vec2 nearest_uv(ivec2 pixel, vec2 texture_size)
{
	vec2 uv = floor(pixel) + 0.5f;
	return uv / texture_size;
}

vec4 to_ndc(vec2 uv, float depth)
{
	const float x = uv.x * 2.0f - 1.0f;
	const float y = (1.0f - uv.y) * 2.0f - 1.0f;
	const float z = depth;
	return vec4(x, y, z, 1.0f);
}

vec3 to_world_pos(vec2 uv, float depth, mat4 inverse_view_proj)
{
	vec4 h = to_ndc(uv, depth);
	vec4 d = inverse_view_proj * h;
	return d.xyz / d.w;
}

// Samples a texture with Catmull-Rom filtering, using 9 texture fetches instead of 16.
// See http://vec3.ca/bicubic-filtering-in-fewer-taps/ for more details
vec3 texture_catmull_rom(vec2 uv, int texture_index, vec2 resolution)
{
	// We're going to sample a a 4x4 grid of texels surrounding the target UV coordinate. We'll do this by rounding
    // down the sample location to get the exact center of our "starting" texel. The starting texel will be at
    // location [1, 1] in the grid, where [0, 0] is the top left corner.
    vec2 sample_position = uv * resolution;
    vec2 tex_pos_1 = floor(sample_position - 0.5f) + 0.5f;
    vec2 inv_resolution = 1.0f / resolution;

    // Compute the fractional offset from our starting texel to our original sample location, which we'll
    // feed into the Catmull-Rom spline function to get our filter weights.
    vec2 f = sample_position - tex_pos_1;

    // Compute the Catmull-Rom weights using the fractional offset that we calculated earlier.
    // These equations are pre-expanded based on our knowledge of where the texels will be located,
    // which lets us avoid having to evaluate a piece-wise function.
    vec2 w0 = f * (-0.5f + f * (1.0f - 0.5f * f));
    vec2 w1 = 1.0f + f * f * (-2.5f + 1.5f * f);
    vec2 w2 = f * (0.5f + f * (2.0f - 1.5f * f));
    vec2 w3 = f * f * (-0.5f + 0.5f * f);

    // Work out weighting factors and sampling offsets that will let us use bilinear filtering to
    // simultaneously evaluate the middle 2 samples from the 4x4 grid.
    vec2 w12 = w1 + w2;
    vec2 offset_12 = w2 / w12;

    // Compute the final UV coordinates we'll use for sampling the texture
    vec2 tex_pos_0 = tex_pos_1 - 1;
    vec2 tex_pos_3 = tex_pos_1 + 2;
    vec2 tex_pos_12 = tex_pos_1 + offset_12;

    tex_pos_0 *= inv_resolution;
    tex_pos_3 *= inv_resolution;
    tex_pos_12 *= inv_resolution;

    vec3 result = vec3(0);

    result += textureLod(textures_2D[nonuniformEXT(texture_index)], vec2(tex_pos_0.x, tex_pos_0.y), 0).rgb * w0.x * w0.y;
    result += textureLod(textures_2D[nonuniformEXT(texture_index)], vec2(tex_pos_12.x, tex_pos_0.y), 0).rgb * w12.x * w0.y;
    result += textureLod(textures_2D[nonuniformEXT(texture_index)], vec2(tex_pos_3.x, tex_pos_0.y), 0).rgb * w3.x * w0.y;

    result += textureLod(textures_2D[nonuniformEXT(texture_index)], vec2(tex_pos_0.x, tex_pos_12.y), 0).rgb * w0.x * w12.y;
    result += textureLod(textures_2D[nonuniformEXT(texture_index)], vec2(tex_pos_12.x, tex_pos_12.y), 0).rgb * w12.x * w12.y;
    result += textureLod(textures_2D[nonuniformEXT(texture_index)], vec2(tex_pos_3.x, tex_pos_12.y), 0).rgb * w3.x * w12.y;

    result += textureLod(textures_2D[nonuniformEXT(texture_index)], vec2(tex_pos_0.x, tex_pos_3.y), 0).rgb * w0.x * w3.y;
    result += textureLod(textures_2D[nonuniformEXT(texture_index)], vec2(tex_pos_12.x, tex_pos_3.y), 0).rgb * w12.x * w3.y;
    result += textureLod(textures_2D[nonuniformEXT(texture_index)], vec2(tex_pos_3.x, tex_pos_3.y), 0).rgb * w3.x * w3.y;

    return result;
}