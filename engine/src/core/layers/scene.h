#pragma once

#include <type_traits>

#include <minimal.h>
#include <core/simulation_layer.h>
#include <core/subsystem.h>

namespace Sunset
{
	class Scene : public SimulationLayer
	{
		public:
			Scene() = default;
			~Scene() = default;

			virtual void initialize() override;
			virtual void update(double delta_time) override;

			template<class T>
			T* get_subsystem()
			{
				assert((std::is_base_of<Subsystem, T>::value));
				auto found = std::find_if(
					subsystems.cbegin(),
					subsystems.cend(),
					[](const std::unique_ptr<Subsystem>& subsystem)
					{ 
						return typeid(*subsystem) == typeid(T);
					}
				);
				return found != subsystems.cend() ? (*found).get() : nullptr;
			}

		protected:
			std::vector<std::unique_ptr<Subsystem>> subsystems;
	};
}
