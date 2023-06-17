#pragma once

#include <minimal.h>
#include <utility/pattern/singleton.h>
#include <physics/physics_types.h>

namespace Sunset
{
	class Physics : public Singleton<Physics>
	{
		friend class Singleton;

	public:
		void initialize();
		void update();
		void destroy();

		class PhysicsContext* context() const
		{
			return physics_context.get();
		}
		
	private:
		Physics() = default;
		Physics(Physics&& other) = delete;
		Physics(const Physics& other) = delete;
		Physics& operator=(const Physics& other) = delete;
		~Physics() = default;

	protected:
		std::unique_ptr<class PhysicsContext> physics_context;
	};
}
