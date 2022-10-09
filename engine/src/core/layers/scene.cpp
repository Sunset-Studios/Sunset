#include <core/layers/scene.h>

namespace Sunset
{
	void Scene::initialize()
	{

	}

	void Scene::update(double delta_time)
	{
		for (auto it = subsystems.begin(); it != subsystems.end(); ++it)
		{
			(*it)->update(delta_time);
		}
	}
}
