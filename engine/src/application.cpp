#include <application.h>
#include <window/window.h>
#include <graphics/renderer.h>

#include <SDL.h>
#include <SDL_vulkan.h>

namespace Sunset
{
	void Application::init()
	{
		bIsInitialized = true;
		window = WindowFactory::create(ENGINE_NAME, glm::ivec2(0), glm::ivec2(1280, 720));
		renderer = RendererFactory::create(window);
	}
	void Application::cleanup()
	{	
		if (bIsInitialized)
		{
			renderer->destroy();
			window->destroy();
		}
	}

	void Application::draw()
	{
		if (bIsInitialized)
		{
			renderer->draw();
		}
	}

	void Application::run()
	{
		if (bIsInitialized)
		{
			bool bQuit = false;
			//main loop
			while (!bQuit && !window->is_closing())
			{
				draw();
			}
		}
	}
}

