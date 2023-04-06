#include <core/ecs/components/light_component.h>
#include <core/data_globals.h>
#include <core/layers/scene.h>

namespace Sunset
{
	LightComponent::LightComponent()
	{
		light = LightGlobals::get()->new_shared_data();
		light_data_buffer_offset = LightGlobals::get()->get_shared_data_index(light);
	}

	LightComponent::~LightComponent()
	{
		LightGlobals::get()->release_shared_data(light);
	}

	void set_light_color(LightComponent* light_comp, const glm::vec3& new_color)
	{
		light_comp->light->color = glm::vec4(new_color, 1.0f);
		LightGlobals::get()->light_dirty_states.set(light_comp->light_data_buffer_offset);
	}

	void set_light_color(class Scene* scene, EntityID entity, const glm::vec3& new_color)
	{
		if (LightComponent* const light_comp = scene->get_component<LightComponent>(entity))
		{
			set_light_color(light_comp, new_color);
		}
	}

	void set_light_direction(LightComponent* light_comp, const glm::vec3& new_direction)
	{
		light_comp->light->direction = glm::vec4(new_direction, light_comp->light->direction.w);
		LightGlobals::get()->light_dirty_states.set(light_comp->light_data_buffer_offset);
	}

	void set_light_direction(class Scene* scene, EntityID entity, const glm::vec3& new_direction)
	{
		if (LightComponent* const light_comp = scene->get_component<LightComponent>(entity))
		{
			set_light_direction(light_comp, new_direction);
		}
	}

	void set_light_radius(LightComponent* light_comp, float radius)
	{
		light_comp->light->radius = radius;
		LightGlobals::get()->light_dirty_states.set(light_comp->light_data_buffer_offset);
	}

	void set_light_radius(class Scene* scene, EntityID entity, float radius)
	{
		if (LightComponent* const light_comp = scene->get_component<LightComponent>(entity))
		{
			set_light_radius(light_comp, radius);
		}
	}


	void set_light_intensity(LightComponent* light_comp, float intensity)
	{
		light_comp->light->intensity = intensity;
		LightGlobals::get()->light_dirty_states.set(light_comp->light_data_buffer_offset);
	}

	void set_light_intensity(class Scene* scene, EntityID entity, float intensity)
	{
		if (LightComponent* const light_comp = scene->get_component<LightComponent>(entity))
		{
			set_light_intensity(light_comp, intensity);
		}
	}

	void set_light_inner_angle(LightComponent* light_comp, float inner_angle)
	{
		light_comp->light->direction.w = inner_angle;
		LightGlobals::get()->light_dirty_states.set(light_comp->light_data_buffer_offset);
	}

	void set_light_inner_angle(class Scene* scene, EntityID entity, float inner_angle)
	{
		if (LightComponent* const light_comp = scene->get_component<LightComponent>(entity))
		{
			set_light_inner_angle(light_comp, inner_angle);
		}
	}

	void set_light_outer_angle(LightComponent* light_comp, float outer_angle)
	{
		light_comp->light->outer_angle = outer_angle;
		LightGlobals::get()->light_dirty_states.set(light_comp->light_data_buffer_offset);
	}

	void set_light_outer_angle(class Scene* scene, EntityID entity, float outer_angle)
	{
		if (LightComponent* const light_comp = scene->get_component<LightComponent>(entity))
		{
			set_light_outer_angle(light_comp, outer_angle);
		}
	}

	void set_light_area_size(LightComponent* light_comp, const glm::vec2& area_size)
	{
		light_comp->light->area_size = area_size;
		LightGlobals::get()->light_dirty_states.set(light_comp->light_data_buffer_offset);
	}

	void set_light_area_size(class Scene* scene, EntityID entity, const glm::vec2& area_size)
	{
		if (LightComponent* const light_comp = scene->get_component<LightComponent>(entity))
		{
			set_light_area_size(light_comp, area_size);
		}
	}

	void set_light_type(LightComponent* light_comp, LightType type)
	{
		light_comp->light->type = type;
		LightGlobals::get()->light_dirty_states.set(light_comp->light_data_buffer_offset);
	}

	void set_light_type(class Scene* scene, EntityID entity, LightType type)
	{
		if (LightComponent* const light_comp = scene->get_component<LightComponent>(entity))
		{
			set_light_type(light_comp, type);
		}
	}

	void set_light_entity_index(LightComponent* light_comp, uint32_t index)
	{
		light_comp->light->entity_index = index;
		LightGlobals::get()->light_dirty_states.set(light_comp->light_data_buffer_offset);
	}

	void set_light_entity_index(class Scene* scene, EntityID entity, uint32_t index)
	{
		if (LightComponent* const light_comp = scene->get_component<LightComponent>(entity))
		{
			set_light_entity_index(light_comp, index);
		}
	}
}
