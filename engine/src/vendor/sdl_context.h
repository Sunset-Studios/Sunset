#pragma once

#include <bitset>
#include <array>

#include <glm/glm.hpp>
#include <SDL.h>
#include <parallel_hashmap/phmap.h>

#include <input_types.h>

namespace Sunset
{
	extern bool B_SDL_INITIALIZED;
	extern SDL_Event sdl_event;
	extern phmap::flat_hash_map<SDL_Keycode, InputKey> SDL_TO_SUNSET_KEY_MAP;
	extern phmap::flat_hash_map<Uint8, InputKey> SDL_TO_SUNSET_BUTTON_MAP;
	extern phmap::flat_hash_map<Uint8, InputRange> SDL_TO_SUNSET_RANGE_MAP;

	extern void lazy_SDL_init();

	class WindowSDL
	{
	public:
		WindowSDL() = default;

		void initialize(const char* title, const glm::ivec2& position, const glm::ivec2& extent, bool b_headless);
		void poll();
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
	};

	class InputProcessorSDL
	{
		public:
			InputProcessorSDL() = default;

			void initialize();
			void update(class InputContext* context, class Window* window);

		protected:
			std::bitset<NUM_AVAILABLE_KEYS> key_bitmap;
			std::array<float, NUM_AVAILABLE_RANGES> ranges_array;
	};
}