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
