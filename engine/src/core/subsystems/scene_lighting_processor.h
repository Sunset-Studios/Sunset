#pragma once

#include <core/subsystem.h>

namespace Sunset
{
	class SceneLightingProcessor : public Subsystem
	{
	public:
		SceneLightingProcessor() = default;
		~SceneLightingProcessor() = default;

		virtual void initialize(class Scene* scene) override;
		virtual void destroy(class Scene* scene) override { };
		virtual void update(class Scene* scene, double delta_time) override;
	};
}
