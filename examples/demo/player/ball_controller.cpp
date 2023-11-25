#include <player/ball_controller.h>

#include <core/ecs/components/transform_component.h>
#include <core/ecs/components/camera_control_component.h>
#include <core/ecs/components/body_component.h>
#include <utility/maths.h>
#include <input/input_provider.h>

#include <random>

namespace Sunset
{
	float rand_float(float min, float max)
	{
		static std::random_device rd;
		static std::mt19937 rng(rd());
		std::uniform_real_distribution<float> distribution(min, max);
		return distribution(rng);
	}

	glm::vec3 random_direction_in_cone(float max_angle, const glm::vec3& cone_direction)
	{
		const float half_angle = max_angle * 0.5f;
		const float angle = rand_float(-half_angle, half_angle);

		glm::vec3 rotation_axis = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, rotation_axis);
		return glm::normalize(rotation * glm::vec4(cone_direction, 0.0f));
	}

	void BallController::initialize(Scene* scene)
	{
		on_collision_delegate.bind<&BallController::on_collision>(this);
	}

	void BallController::update(Scene* scene, double delta_time)
	{
		CameraControlComponent* const camera_control_comp = scene->get_component<CameraControlComponent>(scene->active_camera);
		const float edge_limit = 15.0f * camera_control_comp->data.aspect_ratio;

		if (ball_entity != 0 && paddle_entity != 0)
		{
			BallComponent* const ball_comp = scene->get_component<BallComponent>(ball_entity);

			BodyComponent* const ball_body_comp = scene->get_component<BodyComponent>(ball_entity);
			BodyComponent* const paddle_body_comp = scene->get_component<BodyComponent>(paddle_entity);

			if (ball_body == 0)
			{
				ball_body = ball_body_comp->body_data.body;
			}

			if (paddle_body == 0)
			{
				paddle_body = paddle_body_comp->body_data.body;
			}

			const bool mouse_pressed = InputProvider::get()->get_action(InputKey::B_mouse_left);
			if (mouse_pressed)
			{
				launch_ball(ball_comp);
			}

			if (ball_comp->should_launch && !ball_comp->in_motion)
			{
				const glm::vec3 random_dir = random_direction_in_cone(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				set_body_velocity(ball_body_comp, ball_comp->speed * random_dir);
				ball_comp->should_launch = false;
				ball_comp->in_motion = true;
			}

			if (paddle_body != 0 && b_nudge_to_paddle_velocity)
			{
				const float paddle_speed = glm::length(paddle_body_comp->body_data.velocity);
				if (paddle_speed > 0.0f)
				{
					const float ball_speed = glm::length(ball_body_comp->body_data.velocity);
					const glm::vec3 paddle_dir = paddle_body_comp->body_data.velocity / glm::max(paddle_speed, 0.00001f);
					const glm::vec3 ball_dir = glm::normalize(ball_body_comp->body_data.velocity) / glm::max(ball_speed, 0.00001f);
					set_body_velocity(ball_body_comp, (ball_speed + speed_increase_increment) * (WORLD_UP + paddle_dir * 0.5f));
				}
				b_nudge_to_paddle_velocity = false;
			}
		}
	}

	void BallController::on_collision(BodyHandle body1, BodyHandle body2)
	{
		if ((body1 == ball_body && body2 == paddle_body)
			|| (body1 == paddle_body && body2 == ball_body))
		{
			b_nudge_to_paddle_velocity = true;
		}
	}

	void set_ball_speed(BallComponent* const ball_comp, float speed)
	{
		ball_comp->speed = speed;
	}

	void set_ball_speed(Scene* const scene, EntityID entity, float speed)
	{
		if (BallComponent* const ball_comp = scene->get_component<BallComponent>(entity))
		{
			set_ball_speed(ball_comp, speed);
		}
	}

	void launch_ball(BallComponent* const ball_comp)
	{
		ball_comp->should_launch = true;
	}

	void launch_ball(Scene* const scene, EntityID entity)
	{
		if (BallComponent* const ball_comp = scene->get_component<BallComponent>(entity))
		{
			launch_ball(ball_comp);
		}
	}
}
