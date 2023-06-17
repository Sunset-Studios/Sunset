#pragma once

#include <common.h>

namespace Sunset
{
	template<class Policy>
	class GenericPhysicsBody
	{
	public:
		GenericPhysicsBody() = default;

		void initialize()
		{
			body_policy.initialize();
		}

		void destroy()
		{
			body_policy.destroy();
		}

	private:
		Policy body_policy;
	};

	class NoopPhysicsBody
	{
	public:
		NoopPhysicsBody() = default;

		void initialize()
		{ }

		void destroy()
		{ }
	};

#if USE_JOLT_PHYSICS
	class PhysicsBody : public GenericPhysicsBody<JoltBody>
	{ };
#else
	class PhysicsBody : public GenericPhysicsBody<NoopPhysicsBody>
	{ };
#endif

	class PhysicsBodyFactory
	{
	public:
		template<typename ...Args>
		static std::unique_ptr<PhysicsBody> create(Args&&... args)
		{
			PhysicsBody* body = new PhysicsBody;
			body->initialize(std::forward<Args>(args)...);
			return std::unique_ptr<PhysicsBody>(body);
		}
	};
}
