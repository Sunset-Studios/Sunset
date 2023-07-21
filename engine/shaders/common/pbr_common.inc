// microfacet distribution
float d_ggx(float NoH, float roughness)
{
	float a = NoH * roughness;
	float k = roughness / (1.0f - NoH * NoH + a * a);
	return k * k * (1.0f / PI);
}

// visibility
float v_smith_ggx_height_correlated(float NoV, float NoL, float roughness)
{
	float a2 = roughness * roughness;
	float ggx_v = NoL * sqrt(NoV * NoV * (1.0f - a2) + a2);
	float ggx_l = NoV * sqrt(NoL * NoL * (1.0f - a2) + a2);
	return 0.5f / (ggx_v + ggx_l);
}

float v_smith_ggx_height_correlated_fast(float NoV, float NoL, float roughness)
{
	return 0.5f / mix(2.0f * NoL * NoV, NoL + NoV, roughness);
}

vec3 importance_sample_ggx(vec2 xi, vec3 n, float roughness)
{
	float a = roughness * roughness;
	float phi = 2.0 * PI * xi.x;
	float cos_theta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
	float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

	vec3 h;
	h.x = cos(phi) * sin_theta;
	h.y = sin(phi) * sin_theta;
	h.z = cos_theta;

	vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, n));
	vec3 bitangent = cross(n, tangent);

	vec3 sample_vec = tangent * h.x + bitangent * h.y + n * h.z;
	return normalize(sample_vec);
}

// fresnel
vec3 f_schlick(float VoH, vec3 f0)
{
	float f = pow(1.0f - VoH, 5.0f);
	return f + f0 * (1.0f - f);
}

float f_schlick(float f0, float f90, float VoH)
{
	float one_minus_voh = 1.0f - VoH;
	float one_minus_voh_2 = one_minus_voh * one_minus_voh;
	return f0 + (f90 - f0) * one_minus_voh_2 * one_minus_voh_2 * one_minus_voh;
}

vec3 f_schlick(const vec3 f0, float f90, float VoH)
{
	float one_minus_voh = 1.0f - VoH;
	float one_minus_voh_2 = one_minus_voh * one_minus_voh;
    return f0 + (f90 - f0) * one_minus_voh_2 * one_minus_voh_2 * one_minus_voh;
}

vec3 f_schlick_roughness(float NoV, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(clamp(1.0 - NoV, 0.0, 1.0), 5.0);
}

// diffuse
float fd_lambert()
{
	return 1.0f / PI;
}

// clear coat
float v_kelemen(float LoH)
{
	return clamp(0.25f / (LoH * LoH), 0.0f, 1.0f);
}
