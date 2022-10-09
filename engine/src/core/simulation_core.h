#pragma once

#include <minimal.h>
#include <singleton.h>

namespace Sunset
{
	class SimulationCore : public Singleton<SimulationCore>
	{
		friend class Singleton;

		public:
			void initialize();
			void update();
			void register_layer(std::unique_ptr<class SimulationLayer>&& layer);

		private:
			SimulationCore() = default;

		private:
			double previous_time{ 0.0f };
			double delta_time{ 0.0f };
			std::vector<std::unique_ptr<class SimulationLayer>> layers;
	};
}
