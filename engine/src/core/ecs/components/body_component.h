#pragma once

#include <core/layers/scene.h>
#include <core/ecs/entity.h>
#include <physics/physics_types.h>
#include <physics/physics.h>
#include <physics/physics_context.h>

namespace Sunset
{
	namespace PhysicsBodyDirtyFlags
	{
		constexpr uint16_t BODY = 0x00000000;
		constexpr uint16_t POSITION = 0x00000001;
		constexpr uint16_t ROTATION = 0x00000002;
		constexpr uint16_t BODY_TYPE = 0x00000004;
		constexpr uint16_t GRAVITY_SCALE = 0x00000008;
	}

	struct PhysicsBodyData
	{
		BodyHandle body{ -1 };	
		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
		PhysicsBodyType body_type{ PhysicsBodyType::Static };
		float gravity_scale{ 1.0f };
		uint16_t dirty_flags{ 0 };
	};

	struct BodyComponent
	{
		PhysicsBodyData body_data;
	};

	template<typename T>
	void set_body_shape(BodyComponent* body_comp, T shape_data)
	{
		assert(body_comp != nullptr && "Cannot set body shape on a null body component!");
		if (body_comp->body_data.body >= 0)
		{
			Physics::get()->context()->destroy_body(body_comp->body_data.body);
		}
		body_comp->body_data.body = Physics::get()->context()->create_body(shape_data, body_comp->body_data.position, body_comp->body_data.rotation, body_comp->body_data.body_type);

		body_comp->body_data.dirty_flags |= PhysicsBodyDirtyFlags::BODY;
	}
	void set_body_position(BodyComponent* body_comp, const glm::vec3& position);
	void set_body_rotation(BodyComponent* body_comp, const glm::quat& rotation);
	void set_body_type(BodyComponent* body_comp, PhysicsBodyType body_type);
	void set_body_gravity_scale(BodyComponent* body_comp, float gravity_scale);

	template<typename T>
	void set_body_shape(class Scene* scene, EntityID entity, T shape_data)
	{
		if (BodyComponent* const body_comp = scene->get_component<BodyComponent>(entity))
		{
			set_body_shape(body_comp, shape_data);
		}
	}
	void set_body_position(class Scene* scene, EntityID entity, const glm::vec3& position);
	void set_body_rotation(class Scene* scene, EntityID entity, const glm::quat& rotation);
	void set_body_type(class Scene* scene, EntityID entity, PhysicsBodyType body_type);
	void set_body_gravity_scale(class Scene* scene, EntityID entity, float gravity_scale);
}
