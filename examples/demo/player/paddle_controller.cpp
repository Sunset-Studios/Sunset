#include <player/paddle_controller.h>
#include <core/ecs/components/transform_component.h>
#include <core/ecs/components/body_component.h>
#include <core/ecs/components/camera_control_component.h>
#include <utility/maths.h>

#include <input/input_provider.h>

namespace Sunset
{
	void PaddleController::update(Scene* scene, double delta_time)
	{
		CameraControlComponent* const camera_control_comp = scene->get_component<CameraControlComponent>(scene->active_camera);
		const float edge_limit = 35.0f;

		if (paddle_entity != 0)
		{
			PaddleComponent* const paddle_comp = scene->get_component<PaddleComponent>(paddle_entity);

			TransformComponent* const transform_comp = scene->get_component<TransformComponent>(paddle_entity);
			BodyComponent* const body_comp = scene->get_component<BodyComponent>(paddle_entity);

			if (locked_paddle_y == 0.0f)
			{
				locked_paddle_y = transform_comp->transform.position.y;
			}

			glm::vec3 pos = transform_comp->transform.position;
			pos.y = locked_paddle_y;

			const float x = InputProvider::get()->get_range(InputRange::M_x);
			if (x != 0.0f)
			{
				pos.z = glm::clamp(pos.z - static_cast<float>(x * paddle_comp->speed * delta_time), -edge_limit, edge_limit);
			}
			move_body(body_comp, pos, body_comp->body_data.rotation);
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
