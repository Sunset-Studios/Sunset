#include <player/paddle_controller.h>
#include <core/ecs/components/transform_component.h>
#include <core/ecs/components/camera_control_component.h>
#include <utility/maths.h>

#include <input/input_provider.h>

namespace Sunset
{
	void PaddleController::update(Scene* scene, double delta_time)
	{
		CameraControlComponent* const camera_control_comp = scene->get_component<CameraControlComponent>(scene->active_camera);
		const float edge_limit = 15.0f * camera_control_comp->data.aspect_ratio;

		if (paddle_entity != 0)
		{
			PaddleComponent* const paddle_comp = scene->get_component<PaddleComponent>(paddle_entity);
			TransformComponent* const transform_comp = scene->get_component<TransformComponent>(paddle_entity);

			const float x = InputProvider::get()->get_range(InputRange::M_x);
			if (x != 0.0f)
			{
				glm::vec3 pos = transform_comp->transform.position;
				pos.z = glm::clamp(pos.z - static_cast<float>(x * paddle_comp->speed * delta_time), -edge_limit, edge_limit);
				set_position(transform_comp, pos);
			}
		}
	}

	void set_paddle_speed(PaddleComponent* const paddle_comp, float speed)
	{
		paddle_comp->speed = speed;
	}

	void set_paddle_speed(Scene* const scene, EntityID entity, float speed)
	{
		if (PaddleComponent* const paddle_comp = scene->get_component<PaddleComponent>(entity))
		{
			set_paddle_speed(paddle_comp, speed);
		}
	}
}
