// Adapted from Three.JS sky model, based on "A Practical Analytic Model for Daylight"
// https://github.com/mrdoob/three.js/blob/master/examples/jsm/objects/Sky.js
// https://courses.cs.duke.edu/fall01/cps124/resources/p91-preetham.pdf

#version 460

#include "common/common.inc"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_color;
layout (location = 3) in vec2 in_tex_coord;
layout (location = 4) in vec3 in_tangent;
layout (location = 5) in vec3 in_bitangent;

layout (location = 0) out vec4 out_frag_position;
layout (location = 1) out vec3 out_sun_dir;
layout (location = 2) out float out_sun_e;
layout (location = 3) out float out_sun_fade;
layout (location = 4) out vec3 out_beta_r;
layout (location = 5) out vec3 out_beta_m;

const float e = 2.71828182845904523536028747135266249775724709369995957;
const float ee = 1000.0;
const float cutoff_angle = 1.6110731556870734;
const float steepness = 1.5;
const vec3 total_rayleigh = vec3(5.804542996261093E-6, 1.3562911419845635E-5, 3.0265902468824876E-5);
const vec3 mie_const = vec3(1.8399918514433978E14, 2.7798023919660528E14, 4.0790479543861094E14);

float sun_intensity( float zenith_angle_cos )
{
	zenith_angle_cos = clamp(zenith_angle_cos, -1.0, 1.0);
	return ee * max(0.0, 1.0 - pow( e, -((cutoff_angle - acos( zenith_angle_cos)) / steepness)));
}

vec3 total_mie(float t)
{
	float c = (0.2 * t) * 10E-18;
	return 0.434 * c * mie_const;
}

void main()
{
	const mat4 model_matrix = mat4(
		vec4(1.0, 0.0, 0.0, 0.0),
		vec4(0.0, 1.0, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(camera_data.position.xyz, 1.0)
	);
	const vec4 world_position = model_matrix * vec4(in_position, 1.0);

	out_sun_dir = normalize(scene_lighting_data.sunlight_direction.xyz);
	out_sun_e = sun_intensity(dot(out_sun_dir, UP));
	out_sun_fade = 1.0 - clamp(1.0 - exp((scene_lighting_data.sunlight_direction.y / 450000.0)), 0.0, 1.0);

	float rayleigh_coeff = scene_lighting_data.atmospheric_rayleigh - (1.0 * (1.0 - out_sun_fade));

	out_beta_r = total_rayleigh * rayleigh_coeff;
	out_beta_m = total_mie(scene_lighting_data.atmospheric_turbidity) * scene_lighting_data.mie_coefficient;

	out_frag_position = world_position;

	gl_Position = camera_data.view_proj * world_position;
	gl_Position.z = gl_Position.w;
}