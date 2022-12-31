#include <core/ecs/components/transform_component.h>
#include <core/layers/scene.h>

namespace Sunset
{
	void set_position(TransformComponent* transform_comp, const glm::vec3& new_position)
	{
		transform_comp->transform.position = new_position;
		transform_comp->transform.b_dirty = true;
	}

	void set_position(Scene* scene, EntityID entity, const glm::vec3& new_position)
	{
		if (TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity))
		{
			set_position(transform_comp, new_position);
		}
	}

	void set_rotation(TransformComponent* transform_comp, const glm::vec3& new_rotation)
	{
		transform_comp->transform.rotation = glm::quat(new_rotation);
		transform_comp->transform.b_dirty = true;
	}


	void set_rotation(Scene* scene, EntityID entity, const glm::vec3& new_rotation)
	{
		if (TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity))
		{
			set_rotation(transform_comp, new_rotation);
		}
	}

	void set_scale(TransformComponent* transform_comp, const glm::vec3& new_scale)
	{
		transform_comp->transform.scale = new_scale;
		transform_comp->transform.b_dirty = true;
	}

	void set_scale(Scene* scene, EntityID entity, const glm::vec3& new_scale)
	{
		if (TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity))
		{
			set_scale(transform_comp, new_scale);
		}
	}
}
