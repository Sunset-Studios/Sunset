#include <core/subsystems/transform_processor.h>
#include <core/ecs/components/transform_component.h>
#include <core/layers/scene.h>

namespace Sunset
{
	void TransformProcessor::update(class Scene* scene, double delta_time)
	{
		for (EntityID entity : SceneView<TransformComponent>(*scene))
		{
			TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity);

			if (transform_comp->transform.b_dirty)
			{
				glm::mat4 translation = glm::translate(glm::mat4(1.0f), transform_comp->transform.position);
				glm::mat4 rotation = glm::mat4_cast(glm::normalize(transform_comp->transform.rotation));
				glm::mat4 scale = glm::scale(glm::mat4(1.0f), transform_comp->transform.scale);
				transform_comp->transform.local_matrix = translation * rotation * scale;
				transform_comp->transform.b_dirty = false;
			}
		}
	}
}
