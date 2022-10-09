#include "core/simulation_core.h"
#include "core/simulation_layer.h"

namespace Sunset
{
	void SimulationCore::initialize()
	{
	}

	void SimulationCore::update()
	{
		if (previous_time == 0.0)
		{
			previous_time = SECONDS_TIME;
		}
		const double current_time = SECONDS_TIME;
		delta_time = glm::min(current_time - previous_time, 0.1);
		previous_time = current_time;

		for (auto it = layers.cbegin(); it != layers.cend(); ++it)
		{
			if (SimulationLayer* const layer = (*it).get())
			{
				layer->update(delta_time);
			}
		}
	}

	void SimulationCore::register_layer(std::unique_ptr<SimulationLayer>&& layer)
	{
		if (std::find(layers.cbegin(), layers.cend(), layer) == layers.cend())
		{
			layers.push_back(std::move(layer));
			layers.back().get()->initialize();
		}
	}
}
