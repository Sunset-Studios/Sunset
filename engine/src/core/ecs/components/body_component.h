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
		constexpr uint16_t NONE = 0x00000000;
		constexpr uint16_t BODY = 0x00000001;
		constexpr uint16_t POSITION = 0x00000002;
		constexpr uint16_t ROTATION = 0x00000004;
		constexpr uint16_t BODY_TYPE = 0x00000008;
		constexpr uint16_t GRAVITY_SCALE = 0x00000010;
		constexpr uint16_t RESTITUTION = 0x00000020;
		constexpr uint16_t VELOCITY = 0x00000040;
		constexpr uint16_t FRICTION = 0x00000080;
		constexpr uint16_t LINEAR_DAMPING = 0x00000100;
		constexpr uint16_t ANGULAR_DAMPING = 0x00000200;
		constexpr uint16_t KINEMATIC_MOVE = 0x00000400;
		constexpr uint16_t USER_DATA = 0x00000800;
	}

	struct PhysicsBodyData
	{
		BodyHandle body{ -1 };	
		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 velocity{ 0.0f, 0.0f, 0.0f };
		PhysicsBodyType body_type{ PhysicsBodyType::Static };
		float gravity_scale{ 1.0f };
		float restitution{ 0.0f };
		float friction{ 0.0f };
		float linear_damping{ 0.05f };
		float angular_damping{ 0.05f };
		uint64_t user_data{ 0 };
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
		body_comp->body_data.body = Physics::get()->context()->create_body(shape_data, body_comp->body_data.position, body_comp->body_data.rotation, body_comp->body_data.body_type, body_comp->body_data.linear_damping, body_comp->body_data.angular_damping);

		body_comp->body_data.dirty_flags |= PhysicsBodyDirtyFlags::BODY;
	}
	void set_body_position(BodyComponent* body_comp, const glm::vec3& position);
	void set_body_rotation(BodyComponent* body_comp, const glm::vec3& rotation);
	void set_body_velocity(BodyComponent* body_comp, const glm::vec3& velocity);
	void set_body_type(BodyComponent* body_comp, PhysicsBodyType body_type);
	void set_body_gravity_scale(BodyComponent* body_comp, float gravity_scale);
	void set_body_restitution(BodyComponent* body_comp, float restitution);
	void set_body_friction(BodyComponent* body_comp, float friction);
	void set_body_linear_damping(BodyComponent* body_comp, float linear_damping);
	void set_body_angular_damping(BodyComponent* body_comp, float linear_damping);
	void set_body_user_data(BodyComponent* body_comp, uint64_t user_data);
	void move_body(BodyComponent* body_comp, const glm::vec3& position, const glm::quat& rotation);

	template<typename T>
	void set_body_shape(class Scene* scene, EntityID entity, T shape_data)
	{
		if (BodyComponent* const body_comp = scene->get_component<BodyComponent>(entity))
		{
			set_body_shape(body_comp, shape_data);
		}
	}
	void set_body_position(class Scene* scene, EntityID entity, const glm::vec3& position);
	void set_body_rotation(class Scene* scene, EntityID entity, const glm::vec3& rotation);
	void set_body_velocity(class Scene* scene, EntityID entity, const glm::vec3& velocity);
	void set_body_type(class Scene* scene, EntityID entity, PhysicsBodyType body_type);
	void set_body_gravity_scale(class Scene* scene, EntityID entity, float gravity_scale);
	void set_body_restitution(class Scene* scene, EntityID entity, float restitution);
	void set_body_friction(class Scene* scene, EntityID entity, float friction);
	void set_body_linear_damping(class Scene* scene, EntityID entity, float linear_damping);
	void set_body_angular_damping(class Scene* scene, EntityID entity, float linear_damping);
	void set_body_user_data(class Scene* scene, EntityID entity, uint64_t user_data);
	void move_body(class Scene* scene, EntityID entity, const glm::vec3& position, const glm::quat& rotation);
}
