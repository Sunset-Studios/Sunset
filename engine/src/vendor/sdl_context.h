#pragma once

#include <glm/glm.hpp>
#include <SDL.h>

namespace Sunset
{
	extern bool bSDLInitialized;

	extern void lazy_init();

	class WindowSDL
	{
	public:
		WindowSDL() = default;

		void initialize(const char* title, const glm::ivec2& position, const glm::ivec2& extent);
		bool is_closing();
		void destroy();

		void* get_window_handle() const
		{
			return window_handle;
		}

		glm::ivec2 get_extent() const
		{
			return extent;
		}

		glm::ivec2 get_position() const
		{
			return position;
		}

	private:
		glm::ivec2 extent;
		glm::ivec2 position;
		SDL_Window* window_handle{ nullptr };
		SDL_Event e{ 0 };
	};
}