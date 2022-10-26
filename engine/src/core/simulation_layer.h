#pragma once

namespace Sunset
{
	class SimulationLayer
	{
		public:
			SimulationLayer() = default;
			virtual ~SimulationLayer() = default;

			virtual void initialize();
			virtual void destroy();
			virtual void update(double delta_time);
	};
}
