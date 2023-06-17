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

	void recalculate_transform(TransformComponent* transform_comp)
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), transform_comp->transform.position);
		glm::mat4 rotation = glm::mat4_cast(glm::normalize(transform_comp->transform.rotation));
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), transform_comp->transform.scale);
		transform_comp->transform.local_matrix = translation * rotation * scale;
	}

	void set_scale(Scene* scene, EntityID entity, const glm::vec3& new_scale)
	{
		if (TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity))
		{
			set_scale(transform_comp, new_scale);
		}
	}

	void recalculate_transform(Scene* scene, EntityID entity)
	{
		if (TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity))
		{
			recalculate_transform(transform_comp);
		}
	}
}
