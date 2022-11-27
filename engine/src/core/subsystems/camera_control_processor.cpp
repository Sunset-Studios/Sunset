#include <core/subsystems/camera_control_processor.h>
#include <core/ecs/components/camera_control_component.h>
#include <core/layers/scene.h>

#include <glm/gtc/matrix_transform.hpp>

namespace Sunset
{
	void CameraControlProcessor::update(class Scene* scene, double delta_time)
	{
		for (EntityID entity : SceneView<CameraControlComponent>(*scene))
		{
			CameraControlComponent* const camera_control_comp = scene->get_component<CameraControlComponent>(entity);

			if (camera_control_comp->data.b_dirty)
			{
				camera_control_comp->data.view_matrix = glm::lookAt(
					camera_control_comp->data.position,
					camera_control_comp->data.position + camera_control_comp->data.forward,
					WORLD_UP
				);
				camera_control_comp->data.projection_matrix = glm::perspective(
					glm::radians(camera_control_comp->data.fov),
					camera_control_comp->data.aspect_ratio,
					camera_control_comp->data.near_plane,
					camera_control_comp->data.far_plane
				);
				camera_control_comp->data.projection_matrix[1][1] *= -1;

				camera_control_comp->data.view_projection_matrix = camera_control_comp->data.projection_matrix * camera_control_comp->data.view_matrix;
				camera_control_comp->data.inverse_view_projection_matrix = glm::inverse(camera_control_comp->data.view_projection_matrix);

				camera_control_comp->data.b_dirty = false;
			}
		}
	}
}
