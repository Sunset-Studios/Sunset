#include <game/brick_controller.h>
#include <core/ecs/components/body_component.h>
#include <physics/physics.h>

namespace Sunset
{
	void BrickController::initialize(Scene* scene)
	{
		current_scene = scene;
		bodies_to_remove.reserve(brick_count);
		on_collision_delegate.bind<&BrickController::on_collision>(this);
	}

	void BrickController::update(Scene* scene, double delta_time)
	{
		for (int32_t i = bodies_to_remove.size() - 1; i >= 0; --i)
		{
			EntityID body_ent = Physics::get()->context()->get_body_user_data(bodies_to_remove[i]);

			if (body_ent != 0 && scene->get_component<BrickComponent>(body_ent) != nullptr)
			{
				BodyComponent* const body_comp = scene->get_component<BodyComponent>(body_ent);
				Physics::get()->context()->destroy_body(body_comp->body_data.body);
				scene->destroy_entity(body_ent);
			}

			bodies_to_remove.erase(bodies_to_remove.begin() + i);
		}
	}

	void BrickController::on_collision(BodyHandle body1, BodyHandle body2)
	{
		if (current_scene)
		{
			bodies_to_remove.push_back(body1);
			bodies_to_remove.push_back(body2);
		}
	}
}
