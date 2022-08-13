#pragma once

#include <common.h>

namespace Sunset
{
	template<class WindowPolicy>
	class GenericWindow
	{
		public:
			GenericWindow() = default;

			void initialize(const char* title, const glm::ivec2& position, const glm::ivec2& extent)
			{
				window_policy.initialize(title, position, extent);
			}

			bool is_closing()
			{
				return window_policy.is_closing();
			}

			void destroy()
			{
				window_policy.destroy();
			}

			void* get_window_handle() const
			{
				return window_policy.get_window_handle();
			}

			glm::ivec2 get_extent() const
			{
				return window_policy.get_extent();
			}

			glm::ivec2 get_position() const
			{
				return window_policy.get_position();
			}

		private:
			WindowPolicy window_policy;
	};

	class NoopWindow
	{
		public:
			NoopWindow() = default;

			void initialize(const char* title, const glm::ivec2& position, const glm::ivec2& extent)
			{ }

			bool is_closing()
			{
				return false;
			}

			void destroy()
			{ }

			void* get_window_handle() const
			{
				return nullptr;
			}

			glm::ivec2 get_extent() const
			{
				return glm::ivec2(0);
			}

			glm::ivec2 get_position() const
			{
				return glm::ivec2(0);
			}
	};

#if USE_SDL_WINDOWING
	class Window : public GenericWindow<WindowSDL>
	{ };
#else
	class Window : public GenericWindow<NoopWindow>
	{ };
#endif

	class WindowFactory
	{
	public:
		template<typename ...Args>
		static Window* create(Args&&... args)
		{
			Window* win = new Window;
			win->initialize(std::forward<Args>(args)...);
			return win;
		}
	};
}