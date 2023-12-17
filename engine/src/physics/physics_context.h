#pragma once

#include <common.h>

namespace Sunset
{
	template<class Policy>
	class GenericPhysicsContext
	{
	public:
		GenericPhysicsContext() = default;

		void initialize()
		{
			physics_policy.initialize();
		}
			
		void destroy()
		{
			physics_policy.destroy();
		}

		void step_simulation()
		{
			physics_policy.step_simulation();
		}

		void set_global_gravity(const glm::vec3& g)
		{
			physics_policy.set_global_gravity(g);
		}

		template<typename T>
		BodyHandle create_body(const T& shape_desc, const glm::vec3& position, const glm::quat& rotation, PhysicsBodyType body_type, float linear_damping, float angular_damping)
		{
			return physics_policy.create_body(shape_desc, position, rotation, body_type, linear_damping, angular_damping);
		}

		void destroy_body(BodyHandle body)
		{
			physics_policy.destroy_body(body);
		}

		void add_body_to_simulation(BodyHandle body)
		{
			physics_policy.add_body_to_simulation(body);
		}

		void remove_body_from_simulation(BodyHandle body)
		{
			physics_policy.remove_body_from_simulation(body);
		}

		void set_body_position(BodyHandle body, const glm::vec3& position)
		{
			physics_policy.set_body_position(body, position);
		}

		void set_body_rotation(BodyHandle body, const glm::quat& rotation)
		{
			physics_policy.set_body_rotation(body, rotation);
		}

		void set_body_velocity(BodyHandle body, const glm::vec3& velocity)
		{
			physics_policy.set_body_velocity(body, velocity);
		}

		void set_body_type(BodyHandle body, PhysicsBodyType body_type)
		{
			physics_policy.set_body_type(body, body_type);
		}

		void set_body_gravity_scale(BodyHandle body, float gravity_scale)
		{
			physics_policy.set_body_gravity_scale(body, gravity_scale);
		}

		void set_body_restitution(BodyHandle body, float restitution)
		{
			physics_policy.set_body_restitution(body, restitution);
		}

		void set_body_friction(BodyHandle body, float friction)
		{
			physics_policy.set_body_friction(body, friction);
		}

		void set_body_active(BodyHandle body)
		{
			physics_policy.set_body_active(body);
		}

		void set_body_inactive(BodyHandle body)
		{
			physics_policy.set_body_inactive(body);
		}

		void set_body_user_data(BodyHandle body, uint64_t data)
		{
			physics_policy.set_body_user_data(body, data);
		}
		
		glm::vec3 get_body_position(BodyHandle body)
		{
			return physics_policy.get_body_position(body);
		}

		glm::quat get_body_rotation(BodyHandle body)
		{
			return physics_policy.get_body_rotation(body);
		}

		glm::vec3 get_body_velocity(BodyHandle body)
		{
			return physics_policy.get_body_velocity(body);
		}

		uint64_t get_body_user_data(BodyHandle body)
		{
			return physics_policy.get_body_user_data(body);
		}

		bool get_body_in_simulation(BodyHandle body)
		{
			return physics_policy.get_body_in_simulation(body);
		}

		void move_body(BodyHandle body, const glm::vec3& new_position, const glm::quat& new_rotation)
		{
			physics_policy.move_body(body, new_position, new_rotation);
		}

	private:
		Policy physics_policy;
	};

	class NoopPhysicsContext
	{
	public:
		NoopPhysicsContext() = default;

		void initialize()
		{ }

		void destroy()
		{ }

		void step_simulation()
		{ }

		void set_global_gravity(const glm::vec3& g)
		{ }

		template<typename T>
		BodyHandle create_body(const T& shape_desc, const glm::vec3& position, const glm::quat& rotation, PhysicsBodyType body_type, float linear_damping, float angular_damping)
		{
			return -1; 
		}

		void destroy_body(BodyHandle body)
		{ }

		void add_body_to_simulation(BodyHandle body)
		{ }

		void remove_body_from_simulation(BodyHandle body)
		{ }

		void set_body_position(BodyHandle body, const glm::vec3& position)
		{ }

		void set_body_rotation(BodyHandle body, const glm::quat& rotation)
		{ }

		void set_body_velocity(BodyHandle body, const glm::vec3& velocity)
		{ }

		void set_body_type(BodyHandle body, PhysicsBodyType body_type)
		{ }

		void set_body_gravity_scale(BodyHandle body, float gravity_scale)
		{ }

		void set_body_restitution(BodyHandle body, float restitution)
		{ }

		void set_body_friction(BodyHandle body, float friction)
		{ }

		void set_body_active(BodyHandle body)
		{ }

		void set_body_inactive(BodyHandle body)
		{ }

		void set_body_user_data(BodyHandle body, uint64_t data)
		{ }

		glm::vec3 get_body_position(BodyHandle body)
		{
			return glm::vec3();
		}

		glm::quat get_body_rotation(BodyHandle body)
		{
			return glm::quat();
		}

		glm::vec3 get_body_velocity(BodyHandle body)
		{
			return glm::vec3();
		}

		uint64_t get_body_user_data(BodyHandle body)
		{
			return 0;
		}

		bool get_body_in_simulation(BodyHandle body)
		{
			return false;
		}

		void move_body(BodyHandle body, const glm::vec3& new_position, const glm::quat& new_rotation)
		{ }
	};

#if USE_JOLT_PHYSICS
	class PhysicsContext : public GenericPhysicsContext<JoltContext>
	{ };
#else
	class PhysicsContext : public GenericPhysicsContext<NoopPhysicsContext>
	{ };
#endif

	class PhysicsContextFactory
	{
	public:
		template<typename ...Args>
		static std::unique_ptr<PhysicsContext> create(Args&&... args)
		{
			PhysicsContext* phsx = new PhysicsContext;
			phsx->initialize(std::forward<Args>(args)...);
			return std::unique_ptr<PhysicsContext>(phsx);
		}
	};
}
