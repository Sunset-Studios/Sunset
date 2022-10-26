#pragma once

namespace Sunset
{
	class Subsystem
	{
	public:
		Subsystem() = default;
		virtual ~Subsystem() = default;

		virtual void initialize(class Scene* scene) = 0;
		virtual void destroy(class Scene* scene) = 0;
		virtual void update(class Scene* scene, double delta_time) = 0;
	};
}
