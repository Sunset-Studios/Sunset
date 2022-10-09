#pragma once

namespace Sunset
{
	class Subsystem
	{
	public:
		Subsystem() = default;
		virtual ~Subsystem() = default;

		virtual void initialize() = 0;
		virtual void update(double delta_time) = 0;
	};
}
