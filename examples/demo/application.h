// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>
#include <glm/glm.hpp>

#include <core/common.h>

namespace Sunset
{
	class Application
	{
		public:
			void init();

			void cleanup();

			void run();

		public:
			void load_scene_objects(class Scene* const scene);

		private:
			class Window* window{ nullptr };
	};
}
