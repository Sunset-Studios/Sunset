#include <physics/physics.h>
#include <physics/physics_context.h>

namespace Sunset
{
	void Physics::initialize()
	{
		if (physics_context == nullptr)
		{
			physics_context = std::make_unique<PhysicsContext>();
			physics_context->initialize();
		}
	}

	void Physics::update()
	{
		if (physics_context != nullptr)
		{
			physics_context->step_simulation();
		}
	}

	void Physics::destroy()
	{
		if (physics_context != nullptr)
		{
			physics_context->destroy();
			physics_context.reset();
		}
	}
}
