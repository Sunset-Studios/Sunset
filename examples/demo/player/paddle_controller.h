#pragma once

#include <core/subsystem.h>
#include <core/layers/scene.h>

namespace Sunset
{
	struct PaddleComponent 
	{
		float speed{ 0.0f };
	};

	void set_paddle_speed(PaddleComponent* const paddle_comp, float speed);
	void set_paddle_speed(Scene* const scene, EntityID entity, float speed);

	class PaddleController : public Subsystem
	{
	public:
		PaddleController() = default;
		~PaddleController() = default;

		virtual void initialize(class Scene* scene) override { };
		virtual void destroy(class Scene* scene) override { };
		virtual void update(class Scene* scene, double delta_time) override;

		inline void set_paddle(EntityID paddle) { paddle_entity = paddle; }

	protected:
		EntityID paddle_entity{ 0 };
	};
}
