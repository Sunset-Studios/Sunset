#pragma once

#include <minimal.h>
#include <core/subsystem.h>

namespace Sunset
{
	class LightProcessor : public Subsystem
	{
	public:
		LightProcessor() = default;
		~LightProcessor() = default;

		virtual void initialize(class Scene* scene) override;
		virtual void destroy(class Scene* scene) override { };
		virtual void update(class Scene* scene, double delta_time) override;
	};
}
