// Adapted from Three.JS sky model, based on "A Practical Analytic Model for Daylight"
// https://github.com/mrdoob/three.js/blob/master/examples/jsm/objects/Sky.js
// https://courses.cs.duke.edu/fall01/cps124/resources/p91-preetham.pdf

#version 460

#include "common/common.inc"
#include "common/postprocess_common.inc"

layout (location = 0) in vec4 in_frag_position;
layout (location = 1) in vec3 in_sun_dir;
layout (location = 2) in float in_sun_e;
layout (location = 3) in float in_sun_fade;
layout (location = 4) in vec3 in_beta_r;
layout (location = 5) in vec3 in_beta_m;

layout (location = 0) out vec4 out_frag_color;

const float three_over_sixteen_pi = 0.05968310365946075;
const float one_over_four_pi = 0.07957747154594767;
const float rayleigh_zenith_length = 8.4E3;
const float mie_zenith_length = 1.25E3;
const float white_scale = 1.0748724675633854; // 1.0 / u2_filmic_tonemapping(1000.0)

float rayleigh_phase(float cos_theta)
{
	return three_over_sixteen_pi * (1.0 + pow(cos_theta, 2.0));
}

float hg_phase(float cos_theta, float g)
{
	float g2 = g * g;
	float inverse = 1.0 / pow(1.0 - 2.0 * g * cos_theta + g2, 1.5);
	return one_over_four_pi * ((1.0 - g2) * inverse);
}

void main()
{
	// setup
	vec3 view_dir = normalize(in_frag_position.xyz - camera_data.position.xyz);
	// optical length
	float zenith_angle = acos(max(0.0, dot(UP, view_dir)));
	float inverse = 1.0 / (cos( zenith_angle ) + 0.15 * pow(93.885 - ((zenith_angle * 180.0) / PI), -1.253));
	float sr = rayleigh_zenith_length * inverse;
	float sm = mie_zenith_length * inverse;

	// combined extinction factor
	vec3 f_ex = exp(-(in_beta_r * sr + in_beta_m * sm));

	// inscattering
	float cos_theta = dot(view_dir, in_sun_dir);

	float r_phase = rayleigh_phase(cos_theta * 0.5 + 0.5);
	vec3 beta_r_theta = in_beta_r * r_phase;

	float m_phase = hg_phase(cos_theta, scene_lighting_data.mie_directional_g);
	vec3 beta_m_theta = in_beta_m * m_phase;

	vec3 beta_delta = (beta_r_theta + beta_m_theta) / (in_beta_r + in_beta_m);
	vec3 lin = pow(in_sun_e * beta_delta * (1.0 - f_ex), vec3(1.5));
	lin *= mix(vec3(1.0), pow(in_sun_e * beta_delta * f_ex, vec3(0.5)), clamp(pow(1.0 - dot(UP, in_sun_dir), 5.0), 0.0, 1.0));

	// nightsky
	float theta = acos(view_dir.y); // elevation --> y-axis, [-pi/2, pi/2]
	float phi = atan(view_dir.z, view_dir.x); // azimuth --> x-axis [-pi/2, pi/2]
	vec2 uv = vec2(phi, theta) / vec2(2.0 * PI, PI) + vec2(0.5, 0.0);
	vec3 l0 = vec3(0.1) * f_ex;

	// composition + solar disc
	float sundisk = smoothstep(scene_lighting_data.sunlight_angular_radius, scene_lighting_data.sunlight_angular_radius + 0.00002, cos_theta);
	l0 += (in_sun_e * 19000.0 * f_ex) * sundisk;

	vec3 tex_color = (lin + l0) * 0.04 + vec3(0.0, 0.0003, 0.00075);

	vec3 curr = u2_filmic_tonemapping(tex_color, log2(2.0 / pow(scene_lighting_data.sunlight_intensity, 4.0)));
	vec3 color = curr * white_scale;

	color = pow(color, vec3(1.0 / (1.2 + (1.2 * in_sun_fade))));

	out_frag_color = vec4(color, 1.0f);
}