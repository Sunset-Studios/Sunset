#include <vendor/sdl_context.h>

#include <SDL_vulkan.h>

namespace Sunset
{
	bool bSDLInitialized = false;

	void lazy_init()
	{
		if (!bSDLInitialized)
		{
			bSDLInitialized = true;
			SDL_Init(SDL_INIT_VIDEO);
		}
	}

	void WindowSDL::initialize(const char* title, const glm::ivec2& position, const glm::ivec2& extent)
	{
		lazy_init();

		SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

		this->extent = extent;
		this->position = position;

		window_handle = SDL_CreateWindow(
			title,
			position.x > 0 ? position.x : SDL_WINDOWPOS_CENTERED,
			position.y > 0 ? position.y : SDL_WINDOWPOS_CENTERED,
			extent.x,
			extent.y,
			window_flags
		);
	}

	bool WindowSDL::is_closing()
	{
		if (SDL_PollEvent(&e) != 0)
		{
			//close the window when user alt-f4s or clicks the X button			
			return e.type == SDL_QUIT;
		}
		return false;
	}

	void WindowSDL::destroy()
	{
		SDL_DestroyWindow(window_handle);
	}
}