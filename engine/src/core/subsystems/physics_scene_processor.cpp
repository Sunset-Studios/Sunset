#include <core/subsystems/physics_scene_processor.h>
#include <core/layers/scene.h>
#include <core/ecs/components/body_component.h>
#include <core/ecs/components/transform_component.h>
#include <core/data_globals.h>
#include <physics/physics.h>
#include <physics/physics_context.h>

namespace Sunset
{
	void PhysicsSceneProcessor::initialize(class Scene* scene)
	{
	}

	void PhysicsSceneProcessor::destroy(Scene* scene)
	{
		Physics::get()->destroy();
	}

	void PhysicsSceneProcessor::update(class Scene* scene, double delta_time)
	{
		PhysicsContext* const phys_context = Physics::get()->context();

		for (EntityID entity : SceneView<BodyComponent, TransformComponent>(*scene))
		{
			BodyComponent* const body_comp = scene->get_component<BodyComponent>(entity);
			TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity);

			// If the body changed entirely (due to shape changes) or if we have explicity set our entity transform data externally, propagate those changes to the physics body instead of the other way around.
			if ((body_comp->body_data.dirty_flags & PhysicsBodyDirtyFlags::BODY) || EntityGlobals::get()->entity_transform_dirty_states.test(get_entity_index(entity)))
			{
				phys_context->set_body_position(body_comp->body_data.body, transform_comp->transform.position);
				phys_context->set_body_rotation(body_comp->body_data.body, transform_comp->transform.rotation);
				body_comp->body_data.dirty_flags &= ~(PhysicsBodyDirtyFlags::BODY);
				continue;
			}

			if (body_comp->body_data.dirty_flags & PhysicsBodyDirtyFlags::POSITION)
			{
				phys_context->set_body_position(body_comp->body_data.body, body_comp->body_data.position);
				body_comp->body_data.dirty_flags &= ~(PhysicsBodyDirtyFlags::POSITION);
			}

			if (body_comp->body_data.dirty_flags & PhysicsBodyDirtyFlags::ROTATION)
			{
				phys_context->set_body_rotation(body_comp->body_data.body, body_comp->body_data.rotation);
				body_comp->body_data.dirty_flags &= ~(PhysicsBodyDirtyFlags::ROTATION);
			}

			if (body_comp->body_data.dirty_flags & PhysicsBodyDirtyFlags::BODY_TYPE)
			{
				phys_context->set_body_type(body_comp->body_data.body, body_comp->body_data.body_type);
				body_comp->body_data.dirty_flags &= ~(PhysicsBodyDirtyFlags::BODY_TYPE);
			}

			if (body_comp->body_data.body_type != PhysicsBodyType::Static)
			{
				// We don't use set_position/set_rotation calls when updating the entity transform with the physics body position/rotation data because we want
				// to directly modify the entity transform without marking it as 'dirty'. Otherwise the code will pick back up in the entity_transform_dirty_states
				// check above in the next frame and we'll be redundantly setting the physics body position unnecessarily.
				phys_context->set_body_active(body_comp->body_data.body);
				body_comp->body_data.position = phys_context->get_body_position(body_comp->body_data.body);
				body_comp->body_data.rotation = phys_context->get_body_rotation(body_comp->body_data.body);
				transform_comp->transform.position = body_comp->body_data.position;
				transform_comp->transform.rotation = body_comp->body_data.rotation;
				recalculate_transform(transform_comp);
			}
		}
		Physics::get()->context()->step_simulation();
	}
}
