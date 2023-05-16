struct LightData
{
	vec4 color;
	vec4 direction;
	float radius;
	float outer_angle;
	vec2 area_size;
	uint type;
	int entity;
	float intensity;
	bool b_use_sun_direction;	
};

vec3 calculate_blinn_phong(
	LightData light,
	vec3 position,
	vec3 normal,
	vec3 view_dir,
	vec3 fragment_pos,
	vec3 albedo,
	float shininess,
	vec3 ambient)
{
	vec3 light_dir;
	float attenuation = 1.0f;

	if (light.type == 0) // Directional
	{
		light_dir = normalize(light.b_use_sun_direction ? -scene_lighting_data.sunlight_direction.xyz : -position);
	}
	else if (light.type == 1) // Point
	{
		const vec3 light_to_frag = fragment_pos - position;
		light_dir = normalize(light_to_frag);
		const float distance = length(light_to_frag);
		const float falloff = 1.0f - smoothstep(0.0f, light.radius, distance);
		attenuation = falloff / (1.0f + 0.09f * distance + 0.032f * distance * distance);
	}
	else if (light.type == 2) // Spot
	{
		const vec3 light_to_frag = fragment_pos - position;
		light_dir = normalize(light_to_frag);
		const float distance = length(light_to_frag);
		const float spot_effect = dot(light_dir, light.direction.xyz);
		const float falloff = pow(spot_effect, 180.0f / max(1.0f, light.outer_angle));
		attenuation = falloff / (1.0f + 0.09f * distance + 0.032f * distance * distance);
	}

	attenuation = clamp(attenuation, 0.0f, 255.0f);

	// Ambient
	vec3 ambient_color = albedo * ambient;

	// Diffuse
	float n_dot_l = max(dot(normal, light_dir), 0.0f);
	vec3 diffuse_color = light.color.rgb * albedo * n_dot_l;

	// Specular
	vec3 halfway = normalize(light_dir + view_dir);
	float n_dot_h = max(dot(normal, halfway), 0.0f);
	float specular = pow(n_dot_h, shininess);
	vec3 specular_color = light.color.rgb * specular;

	// Attenuation
	vec3 final_color = ambient_color + (diffuse_color + specular_color) * light.intensity * attenuation;

	return final_color;
}

vec3 calculate_brdf(
	LightData light,
	vec3 position,
	vec3 normal,
	vec3 view_dir,
	vec3 fragment_pos,
	vec3 albedo,
	float roughness,
	float metallic,
	float reflectance,
	float clear_coat,
	float clear_coat_roughness,
	float ao)
{
	vec3 light_dir;
	float attenuation = 1.0f;

	if (light.type == 0) // Directional
	{
		light_dir = normalize(light.b_use_sun_direction ? -scene_lighting_data.sunlight_direction.xyz : -position);
	}
	else if (light.type == 1) // Point
	{
		const vec3 light_to_frag = fragment_pos - position;
		light_dir = normalize(light_to_frag);
		const float distance_squared = dot(light_to_frag, light_to_frag);
		const float light_inv_radius = 1.0f / light.radius;
		const float factor = distance_squared * light_inv_radius * light_inv_radius;
		const float smooth_factor = max(1.0f - factor * factor, 0.0f);
		attenuation = (smooth_factor * smooth_factor) / max(distance_squared, 0.0001f); 
	}
	else if (light.type == 2) // Spot
	{
		const vec3 light_to_frag = fragment_pos - position;
		light_dir = normalize(light_to_frag);
		float cos_outer = cos(light.outer_angle);
		float spot_scale = 1.0f / max(cos(light.direction.w) - cos_outer, 0.0001f);
		float spot_offset = -cos_outer * spot_scale;
		float cd = dot(normalize(light_dir), vec3(1.0f));
		float attenuation = clamp(cd * spot_scale + spot_offset, 0.0f, 1.0f);
		attenuation = attenuation * attenuation;
	}

	attenuation = clamp(attenuation, 0.0f, 255.0f);

	vec3 halfway = normalize(light_dir + view_dir);

	float n_dot_v = clamp(dot(normal, view_dir), 0.0001f, 1.0f);
	float n_dot_l = clamp(dot(normal, light_dir), 0.0001f, 1.0f);
	float n_dot_h = clamp(dot(normal, halfway), 0.0001f, 1.0f);
	float l_dot_h = clamp(dot(light_dir, halfway), 0.0001f, 1.0f);
	float v_dot_h = clamp(dot(view_dir, halfway), 0.0001f, 1.0f);

	float a = roughness * roughness;

	vec3 lluminance = light.intensity * attenuation * n_dot_l * light.color.rgb;

	// specular reflectance at normal incidence angle for both dielectric and metallic materials
	vec3 f0 = 0.16f * reflectance * reflectance * (1.0f - metallic) + albedo * metallic;
	// account for clear coat interface
	if (clear_coat > 0.0f)
	{
		vec3 f0_clear_coat = clamp(f0 * (f0 * (0.941892 - 0.263008 * f0) + 0.346479) - 0.0285998, vec3(0.0f), vec3(1.0f));
		f0 = mix(f0, f0_clear_coat, clear_coat);
	}
	float f90 = clamp(dot(f0, vec3(50.0 * 0.33)), 0.0f, 1.0f);

	float d = d_ggx(n_dot_h, a);
	vec3 f = f_schlick(f0, f90, l_dot_h);
	float v = v_smith_ggx_height_correlated_fast(n_dot_v, n_dot_l, a);

	// specular BRDF
	vec3 fr = (d * v) * f;

	// diffuse BRDF
	vec3 diffuse_color = (1.0f - metallic) * albedo;
	vec3 fd = diffuse_color * fd_lambert();

	// remapping and linearization of clear coat roughness
	float clamped_clear_coat_roughness = clamp(clear_coat_roughness, 0.089f, 1.0f);
	float cc_roughness = clamped_clear_coat_roughness * clamped_clear_coat_roughness;

	// clear coat BRDG
	// TODO: clear coat should be using geometric normal instead of detail normal
	float dc = d_ggx(cc_roughness, n_dot_h);
	float vc = v_kelemen(l_dot_h);
	float fc = f_schlick(0.04f, 1.0f, l_dot_h) * clear_coat;
	float frc = (dc * vc) * fc;

	// account for energy loss in the base layer
	vec3 brdf = ((fd + fr * (1.0f - fc)) * (1.0f - fc) + frc);
	return brdf * lluminance;
}