#pragma once

#include <core/subsystem.h>
#include <core/layers/scene.h>
#include <physics/physics_types.h>

namespace Sunset
{
	struct BrickComponent
	{ };

	class BrickController : public Subsystem
	{
	public:
		BrickController() = default;
		~BrickController() = default;

		virtual void initialize(class Scene* scene) override;
		virtual void destroy(class Scene* scene) override { };
		virtual void update(class Scene* scene, double delta_time) override;

		inline void set_brick_count(uint32_t count) { brick_count = count; }

	protected:
		void on_collision(BodyHandle body1, BodyHandle body2);

	protected:
		uint32_t brick_count;
		Scene* current_scene{ nullptr };
		std::vector<BodyHandle> bodies_to_remove;
	};
}
