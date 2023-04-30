#pragma once

#include <minimal.h>
#include <graphics/light.h>
#include <core/ecs/entity.h>

namespace Sunset
{
	struct LightComponent
	{
		LightComponent();
		~LightComponent();

		LightData* light{ nullptr };
		uint32_t light_data_buffer_offset{ 0 };
	};

	void set_light_color(LightComponent* light_comp, const glm::vec3& new_color);
	void set_light_direction(LightComponent* light_comp, const glm::vec3& new_direction);
	void set_light_radius(LightComponent* light_comp, float radius);
	void set_light_intensity(LightComponent* light_comp, float intensity);
	void set_light_inner_angle(LightComponent* light_comp, float inner_angle);
	void set_light_outer_angle(LightComponent* light_comp, float outer_angle);
	void set_light_area_size(LightComponent* light_comp, const glm::vec2& area_size);
	void set_light_type(LightComponent* light_comp, LightType type);
	void set_light_entity_index(LightComponent* light_comp, uint32_t index);
	void set_light_should_use_sun_direction(LightComponent* light_comp, bool use_sun_dir);

	void set_light_color(class Scene* scene, EntityID entity, const glm::vec3& new_color);
	void set_light_direction(class Scene* scene, EntityID entity, const glm::vec3& new_direction);
	void set_light_radius(class Scene* scene, EntityID entity, float radius);
	void set_light_intensity(class Scene* scene, EntityID entity, float intensity);
	void set_light_inner_angle(class Scene* scene, EntityID entity, float inner_angle);
	void set_light_outer_angle(class Scene* scene, EntityID entity, float outer_angle);
	void set_light_area_size(class Scene* scene, EntityID entity, const glm::vec2& area_size);
	void set_light_type(class Scene* scene, EntityID entity, LightType type);
	void set_light_entity_index(class Scene* scene, EntityID entity, uint32_t index);
	void set_light_should_use_sun_direction(class Scene* scene, EntityID entity, bool use_sun_dir);
}
