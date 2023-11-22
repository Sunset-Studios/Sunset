#pragma once

#include <minimal.h>
#include <core/delegate.h>

namespace Sunset
{
	using BodyHandle = int64_t;

	constexpr uint32_t MAX_PHYSICS_BODIES = 65536;

	extern MultiDelegate<void(BodyHandle,BodyHandle)> on_collision_delegate;

	enum class PhysicsBodyType : uint16_t
	{
		Static = 0,
		Dynamic,
		Kinematic
	};

	struct SphereShapeDescription
	{
		float radius;
	};

	struct BoxShapeDescription
	{
		glm::vec3 half_extent;
		float convex_radius{ 0.05f };
	};

	struct CapsuleShapeDescription
	{
		float half_height;
		float top_radius;
		float bottom_radius;
	};

	struct CylinderShapeDescription
	{
		float half_height;
		float radius;
		float convex_radius{ 0.05f };
	};
}