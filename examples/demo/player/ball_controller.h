#pragma once

#include <core/subsystem.h>
#include <core/layers/scene.h>
#include <physics/physics_types.h>

namespace Sunset
{
	struct BallComponent 
	{
		float speed{ 0.0f };
		bool should_launch{ false };
		bool in_motion{ false };
	};

	void set_ball_speed(BallComponent* const ball_comp, float speed);
	void set_ball_speed(Scene* const scene, EntityID entity, float speed);

	void launch_ball(BallComponent* const ball_comp);
	void launch_ball(Scene* const scene, EntityID entity);

	class BallController : public Subsystem
	{
	public:
		BallController() = default;
		~BallController() = default;

		virtual void initialize(class Scene* scene) override;
		virtual void destroy(class Scene* scene) override { };
		virtual void update(class Scene* scene, double delta_time) override;

		inline void set_ball(EntityID ball) { ball_entity = ball; }
		inline void set_paddle(EntityID paddle) { paddle_entity = paddle; } 
		inline void set_speed_increase_increment(float increment) { speed_increase_increment = increment; }

	protected:
		void on_collision(BodyHandle body1, BodyHandle body2);

	protected:
		EntityID ball_entity{ 0 };
		EntityID paddle_entity{ 0 };
		BodyHandle ball_body{ 0 };
		BodyHandle paddle_body{ 0 };
		float speed_increase_increment{ 0.1f };
		bool b_nudge_to_paddle_velocity{ false };
	};
}
