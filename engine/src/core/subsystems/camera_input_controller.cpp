#include <core/subsystems/camera_input_controller.h>
#include <core/ecs/components/camera_control_component.h>
#include <core/layers/scene.h>
#include <input/input_provider.h>

namespace Sunset
{
	void CameraInputController::update(class Scene* scene, double delta_time)
	{
		if (CameraControlComponent* const camera_control_comp = scene->get_component<CameraControlComponent>(scene->active_camera))
		{
			glm::vec3 camera_pos(camera_control_comp->data.position);
			glm::vec3 prev_camera_pos(camera_pos);
			if (InputProvider::get()->get_state(InputKey::K_w))
			{
				camera_pos += camera_control_comp->input.move_speed * camera_control_comp->data.forward * static_cast<float>(delta_time);
			}
			if (InputProvider::get()->get_state(InputKey::K_s))
			{
				camera_pos -= camera_control_comp->input.move_speed * camera_control_comp->data.forward * static_cast<float>(delta_time);
			}

			if (InputProvider::get()->get_state(InputKey::K_a))
			{
				camera_pos -= camera_control_comp->input.move_speed * glm::normalize(glm::cross(camera_control_comp->data.forward, WORLD_UP)) * static_cast<float>(delta_time);
			}
			if (InputProvider::get()->get_state(InputKey::K_d))
			{
				camera_pos += camera_control_comp->input.move_speed * glm::normalize(glm::cross(camera_control_comp->data.forward, WORLD_UP)) * static_cast<float>(delta_time);
			}

			if (InputProvider::get()->get_state(InputKey::B_mouse_left))
			{
				float x = InputProvider::get()->get_range(InputRange::M_x);
				float y = InputProvider::get()->get_range(InputRange::M_y);
				if (x != 0.0f || y != 0.0f)
				{
					float& pitch = camera_control_comp->data.pitch;
					float& yaw = camera_control_comp->data.yaw;

					x *= camera_control_comp->input.look_speed * delta_time;
					y *= camera_control_comp->input.look_speed * delta_time;

					yaw += x;
					pitch = glm::clamp(pitch + y, -89.0f, 89.0f);

					glm::vec3 new_forward(
						glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
						glm::sin(glm::radians(pitch)),
						glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch))
					);

					new_forward = glm::mix(new_forward, camera_control_comp->data.prev_forward, 0.5f);

					set_forward(camera_control_comp, glm::normalize(new_forward));
				}
			}

			if (camera_pos != prev_camera_pos)
			{
				camera_pos = glm::mix(camera_pos, camera_control_comp->data.prev_position, 0.5f);
				set_position(camera_control_comp, camera_pos);
			}
		}
	}
}
