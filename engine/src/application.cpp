#include <application.h>
#include <window/window.h>
#include <graphics/renderer.h>
#include <input/input_provider.h>

#include <SDL.h>
#include <SDL_vulkan.h>

namespace Sunset
{
	void Application::init()
	{
		bIsInitialized = true;
		window = WindowFactory::create(ENGINE_NAME, glm::ivec2(0), glm::ivec2(1280, 720));
		renderer = RendererFactory::create(window);

		InputProvider::get()->push_context(InputProvider::default_context());
	}

	void Application::cleanup()
	{	
		if (bIsInitialized)
		{
			renderer->destroy();
			window->destroy();
		}
	}

	void Application::run()
	{
		if (bIsInitialized)
		{
			bool bQuit = false;
			
			while (!bQuit && !window->is_closing())
			{
				window->poll();

				renderer->draw();

				InputProvider::get()->update();
			}
		}
	}
}

