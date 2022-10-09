#pragma once

#include <core/subsystem.h>

namespace Sunset
{
	class StaticMeshProcessor : public Subsystem
	{
	public:
		StaticMeshProcessor() = default;
		~StaticMeshProcessor() = default;

		virtual void initialize() override { };
		virtual void update(double delta_time) override { };
	};
}
