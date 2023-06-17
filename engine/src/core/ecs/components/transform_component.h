#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <core/ecs/entity.h>

namespace Sunset
{
	struct TransformData
	{
		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
		glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
		glm::mat4 local_matrix;
		bool b_dirty{ false };
	};

	struct TransformComponent
	{
		TransformData transform;
	};

	void set_position(TransformComponent* transform_comp, const glm::vec3& new_position);
	void set_rotation(TransformComponent* transform_comp, const glm::vec3& new_rotation);
	void set_scale(TransformComponent* transform_comp, const glm::vec3& new_scale);
	void recalculate_transform(TransformComponent* transform_comp);

	void set_position(class Scene* scene, EntityID entity, const glm::vec3& new_position);
	void set_rotation(class Scene* scene, EntityID entity, const glm::vec3& new_rotation);
	void set_scale(class Scene* scene, EntityID entity, const glm::vec3& new_scale);
	void recalculate_transform(class Scene* scene, EntityID entity);
}
