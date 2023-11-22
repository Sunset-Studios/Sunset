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
		for (EntityID entity : SceneView<BodyComponent, TransformComponent>(*scene))
		{
			BodyComponent* const body_comp = scene->get_component<BodyComponent>(entity);
			Physics::get()->context()->destroy_body(body_comp->body_data.body);
			body_comp->body_data.body = -1;
		}
		Physics::get()->destroy();
	}

	void PhysicsSceneProcessor::update(class Scene* scene, double delta_time)
	{
		ZoneScopedN("PhysicsSceneProcessor::update");

		PhysicsContext* const phys_context = Physics::get()->context();

		for (EntityID entity : SceneView<BodyComponent, TransformComponent>(*scene))
		{
			BodyComponent* const body_comp = scene->get_component<BodyComponent>(entity);
			TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity);

			const bool b_in_simulation = phys_context->get_body_in_simulation(body_comp->body_data.body);
			if (!b_in_simulation || (body_comp->body_data.dirty_flags & PhysicsBodyDirtyFlags::BODY) || EntityGlobals::get()->entity_transform_dirty_states.test(get_entity_index(entity)))
			{
				continue;
			}

			if (body_comp->body_data.dirty_flags & PhysicsBodyDirtyFlags::KINEMATIC_MOVE)
			{
				phys_context->move_body(body_comp->body_data.body, body_comp->body_data.position, body_comp->body_data.rotation);
				body_comp->body_data.dirty_flags &= ~(PhysicsBodyDirtyFlags::KINEMATIC_MOVE);
			}
		}

		phys_context->step_simulation();

		for (EntityID entity : SceneView<BodyComponent, TransformComponent>(*scene))
		{
			BodyComponent* const body_comp = scene->get_component<BodyComponent>(entity);
			TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity);

			const bool b_in_simulation = phys_context->get_body_in_simulation(body_comp->body_data.body);
			if (!b_in_simulation)
			{
				continue;
			}

			// If the body changed entirely (due to shape changes) or if we have explicity set our entity transform data externally, propagate those changes to the physics body instead of the other way around.
			if ((body_comp->body_data.dirty_flags & PhysicsBodyDirtyFlags::BODY) || EntityGlobals::get()->entity_transform_dirty_states.test(get_entity_index(entity)))
			{
				phys_context->set_body_position(body_comp->body_data.body, transform_comp->transform.position);
				phys_context->set_body_rotation(body_comp->body_data.body, transform_comp->transform.rotation);
				phys_context->set_body_active(body_comp->body_data.body);
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

			if (body_comp->body_data.dirty_flags & PhysicsBodyDirtyFlags::VELOCITY)
			{
				phys_context->set_body_velocity(body_comp->body_data.body, body_comp->body_data.velocity);
				body_comp->body_data.dirty_flags &= ~(PhysicsBodyDirtyFlags::VELOCITY);
			}

			if (body_comp->body_data.dirty_flags & PhysicsBodyDirtyFlags::BODY_TYPE)
			{
				phys_context->set_body_type(body_comp->body_data.body, body_comp->body_data.body_type);
				body_comp->body_data.dirty_flags &= ~(PhysicsBodyDirtyFlags::BODY_TYPE);
			}

			if (body_comp->body_data.dirty_flags & PhysicsBodyDirtyFlags::GRAVITY_SCALE)
			{
				phys_context->set_body_gravity_scale(body_comp->body_data.body, body_comp->body_data.gravity_scale);
				body_comp->body_data.dirty_flags &= ~(PhysicsBodyDirtyFlags::GRAVITY_SCALE);
			}

			if (body_comp->body_data.dirty_flags & PhysicsBodyDirtyFlags::RESTITUTION)
			{
				phys_context->set_body_restitution(body_comp->body_data.body, body_comp->body_data.restitution);
				body_comp->body_data.dirty_flags &= ~(PhysicsBodyDirtyFlags::RESTITUTION);
			}

			if (body_comp->body_data.dirty_flags & PhysicsBodyDirtyFlags::FRICTION)
			{
				phys_context->set_body_friction(body_comp->body_data.body, body_comp->body_data.friction);
				body_comp->body_data.dirty_flags &= ~(PhysicsBodyDirtyFlags::FRICTION);
			}

			if (body_comp->body_data.body_type != PhysicsBodyType::Static)
			{
				body_comp->body_data.position = phys_context->get_body_position(body_comp->body_data.body);
				body_comp->body_data.rotation = phys_context->get_body_rotation(body_comp->body_data.body);
				body_comp->body_data.velocity = phys_context->get_body_velocity(body_comp->body_data.body);
				set_position(transform_comp, body_comp->body_data.position);
				set_rotation(transform_comp, glm::eulerAngles(body_comp->body_data.rotation));
			}
		}
	}
}
