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
			//initializes everything in the engine
			void init();

			//shuts down the engine
			void cleanup();

			//run main loop
			void run();

		private:
			bool bIsInitialized{ false };
			int frameNumber{ 0 };
			class Window* window{ nullptr };
			class Renderer* renderer{ nullptr };
	};
}
