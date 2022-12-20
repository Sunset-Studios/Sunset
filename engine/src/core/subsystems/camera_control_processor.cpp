#include <core/subsystems/camera_control_processor.h>
#include <core/ecs/components/camera_control_component.h>
#include <core/layers/scene.h>
#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>

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
				camera_control_comp->data.matrices.view_matrix = glm::lookAt(
					camera_control_comp->data.position,
					camera_control_comp->data.position + camera_control_comp->data.forward,
					WORLD_UP
				);
				camera_control_comp->data.matrices.projection_matrix = glm::perspective(
					glm::radians(camera_control_comp->data.fov),
					camera_control_comp->data.aspect_ratio,
					camera_control_comp->data.near_plane,
					camera_control_comp->data.far_plane
				);
				camera_control_comp->data.matrices.projection_matrix[1][1] *= -1;

				camera_control_comp->data.matrices.view_projection_matrix = camera_control_comp->data.matrices.projection_matrix * camera_control_comp->data.matrices.view_matrix;
				camera_control_comp->data.matrices.inverse_view_projection_matrix = glm::inverse(camera_control_comp->data.matrices.view_projection_matrix);

				camera_control_comp->data.b_dirty = false;

				for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i)
				{
					if (CameraControlComponent::gpu_cam_buffers[i] == nullptr)
					{
						CameraControlComponent::gpu_cam_buffers[i] = BufferFactory::create(Renderer::get()->context(), sizeof(CameraMatrices), BufferType::UniformBuffer);
					}
				}

				{
					const uint32_t current_buffer_idx = Renderer::get()->context()->get_buffered_frame_number();
					CameraControlComponent::gpu_cam_buffers[current_buffer_idx]->copy_from(Renderer::get()->context(), &camera_control_comp->data.matrices, sizeof(CameraMatrices));
				}
			}
		}
	}
}
