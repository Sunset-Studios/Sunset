#include "common/pbr_common.inc"

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
	uint b_use_sun_direction;
	uint b_casts_shadows;
	uint b_csm_caster;
	uint padding[2];	
};

float calculate_csm_shadow(vec3 frag_pos, vec3 normal, vec3 light_dir, int shadow_map_index)
{
	// Select the appropriate cascade
	const vec4 frag_pos_view_space = camera_data.view * vec4(frag_pos, 1.0);
	const float depth = abs(frag_pos_view_space.z);

	vec4 res = step(scene_lighting_data.csm_plane_distances, vec4(depth));
	int cascade = clamp(int(res.x + res.y + res.z), 0, MAX_SHADOW_CASCADES - 1);

	// Calculate projected coords based on light space matrix for the given cascade
	const vec4 frag_pos_light_space = scene_lighting_data.light_space_matrices[cascade] * vec4(frag_pos, 1.0);
	const vec3 proj_coords = (frag_pos_light_space.xyz / frag_pos_light_space.w) * 0.5 + 0.5;

	if (proj_coords.z > 1.0)
	{
		return 0.0;
	}

	float bias = max(0.05 * (1.0 - dot(normal, light_dir)), 0.005);
	bias *= 1.0 / (scene_lighting_data.csm_plane_distances[cascade] * 0.5);

	// PCF
	const vec2 texel_size = 1.0 / textureSize(textures_2DArray[nonuniformEXT(shadow_map_index)], 0).xy;
	const float biased_depth = proj_coords.z - bias;

	float shadow = 0.0;
	float pcf_depth = texture(textures_2DArray[nonuniformEXT(shadow_map_index)], vec3(proj_coords.xy + vec2(-1, -1) * texel_size, cascade)).r;
	shadow += float(biased_depth > pcf_depth);

	pcf_depth = texture(textures_2DArray[nonuniformEXT(shadow_map_index)], vec3(proj_coords.xy + vec2(-1, 0) * texel_size, cascade)).r;
	shadow += float(biased_depth > pcf_depth);

	pcf_depth = texture(textures_2DArray[nonuniformEXT(shadow_map_index)], vec3(proj_coords.xy + vec2(-1, 1) * texel_size, cascade)).r;
	shadow += float(biased_depth > pcf_depth);

	pcf_depth = texture(textures_2DArray[nonuniformEXT(shadow_map_index)], vec3(proj_coords.xy + vec2(0, -1) * texel_size, cascade)).r;
	shadow += float(biased_depth > pcf_depth);

	pcf_depth = texture(textures_2DArray[nonuniformEXT(shadow_map_index)], vec3(proj_coords.xy + vec2(0, 0) * texel_size, cascade)).r;
	shadow += float(biased_depth > pcf_depth);

	pcf_depth = texture(textures_2DArray[nonuniformEXT(shadow_map_index)], vec3(proj_coords.xy + vec2(0, 1) * texel_size, cascade)).r;
	shadow += float(biased_depth > pcf_depth);

	pcf_depth = texture(textures_2DArray[nonuniformEXT(shadow_map_index)], vec3(proj_coords.xy + vec2(1, -1) * texel_size, cascade)).r;
	shadow += float(biased_depth > pcf_depth);

	pcf_depth = texture(textures_2DArray[nonuniformEXT(shadow_map_index)], vec3(proj_coords.xy + vec2(1, 0) * texel_size, cascade)).r;
	shadow += float(biased_depth > pcf_depth);

	pcf_depth = texture(textures_2DArray[nonuniformEXT(shadow_map_index)], vec3(proj_coords.xy + vec2(1, 1) * texel_size, cascade)).r;
	shadow += float(biased_depth > pcf_depth);

	return shadow / 9.0;
}

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
		light_dir = normalize(light.b_use_sun_direction > 0 ? -scene_lighting_data.sunlight_direction.xyz : -position);
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
	float ao,
	vec3 irradiance,
	vec3 prefiltered_color,
	vec2 env_brdf,
	int shadow_map_index)
{
	vec3 light_dir;
	float attenuation = 1.0f;

	if (light.type == 0) // Directional
	{
		light_dir = normalize(light.b_use_sun_direction > 0 ? scene_lighting_data.sunlight_direction.xyz : -position);
	}
	else if (light.type == 1) // Point
	{
		const vec3 light_to_frag = position - fragment_pos;
		light_dir = normalize(light_to_frag);
		const float distance_squared = dot(light_to_frag, light_to_frag);
		const float light_inv_radius = 1.0f / light.radius;
		const float factor = distance_squared * light_inv_radius * light_inv_radius;
		const float smooth_factor = max(1.0f - factor * factor, 0.0f);
		attenuation = (smooth_factor * smooth_factor) / max(distance_squared, 0.0001f); 
	}
	else if (light.type == 2) // Spot
	{
		const vec3 light_to_frag = position - fragment_pos;
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

	vec3 env_f = f_schlick_roughness(n_dot_v, f0, a);

	// diffuse BRDF
	vec3 diffuse_color = (1.0f - metallic) * albedo * irradiance;
	vec3 env_specular = prefiltered_color * (f0 * env_brdf.x + env_brdf.y);
	vec3 fd = (diffuse_color * (1.0f - env_f) * (1.0f - metallic) + env_specular) * ao;

	// remapping and linearization of clear coat roughness
	float clamped_clear_coat_roughness = clamp(clear_coat_roughness, 0.089f, 1.0f);
	float cc_roughness = clamped_clear_coat_roughness * clamped_clear_coat_roughness;

	// clear coat BRDG
	// TODO: clear coat should be using geometric normal instead of detail normal
	float dc = d_ggx(cc_roughness, n_dot_h);
	float vc = v_kelemen(l_dot_h);
	float fc = f_schlick(0.04f, 1.0f, l_dot_h) * clear_coat;
	float frc = (dc * vc) * fc;

	// shadow
	float shadow_factor = light.b_csm_caster > 0 && shadow_map_index != -1 ? calculate_csm_shadow(fragment_pos, normal, light_dir, shadow_map_index) : 0.0;

	// account for energy loss in the base layer
	vec3 brdf = ((fd + (1.0 - shadow_factor) * fr * (1.0f - fc)) * (1.0f - fc) + frc);
	return brdf * lluminance;
}
