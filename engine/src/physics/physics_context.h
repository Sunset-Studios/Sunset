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
		BodyHandle create_body(const T& shape_desc, const glm::vec3& position, const glm::quat& rotation, PhysicsBodyType body_type)
		{
			return physics_policy.create_body(shape_desc, position, rotation, body_type);
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

		void set_body_type(BodyHandle body, PhysicsBodyType body_type)
		{
			physics_policy.set_body_type(body, body_type);
		}

		void set_body_gravity_scale(BodyHandle body, float gravity_scale)
		{
			physics_policy.set_body_gravity_scale(body, gravity_scale);
		}

		void set_body_active(BodyHandle body)
		{
			physics_policy.set_body_active(body);
		}

		void set_body_inactive(BodyHandle body)
		{
			physics_policy.set_body_inactive(body);
		}
		
		glm::vec3 get_body_position(BodyHandle body)
		{
			return physics_policy.get_body_position(body);
		}

		glm::quat get_body_rotation(BodyHandle body)
		{
			return physics_policy.get_body_rotation(body);
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
		BodyHandle create_body(const T& shape_desc, const glm::vec3& position, const glm::quat& rotation, PhysicsBodyType body_type)
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

		void set_body_type(BodyHandle body, PhysicsBodyType body_type)
		{ }

		void set_body_gravity_scale(BodyHandle body, float gravity_scale)
		{ }

		void set_body_active(BodyHandle body)
		{ }

		void set_body_inactive(BodyHandle body)
		{ }

		glm::vec3 get_body_position(BodyHandle body)
		{
			return glm::vec3();
		}

		glm::quat get_body_rotation(BodyHandle body)
		{
			return glm::quat();
		}
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
