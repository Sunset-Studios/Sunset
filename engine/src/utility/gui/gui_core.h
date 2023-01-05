#pragma once

#include <common.h>

namespace Sunset
{
	template<class Policy>
	class GenericGUICore
	{
	public:
		GenericGUICore() = default;

		void initialize(class GraphicsContext* gfx_context, class Window* const window)
		{
			gui_policy.initialize(gfx_context, window);
		}

		void new_frame(class Window* const window)
		{
			gui_policy.new_frame(window);
		}

		void poll_events()
		{
			gui_policy.poll_events();
		}

		void begin_draw()
		{
			gui_policy.begin_draw();
		}

		void end_draw(void* command_buffer)
		{
			gui_policy.end_draw(command_buffer);
		}

	private:
		Policy gui_policy;
	};

	class NoopGUICore
	{
	public:
		NoopGUICore() = default;

		void initialize(class GraphicsContext* gfx_context, class Window* const window)
		{ }

		void new_frame(class Window* const window)
		{ }

		void poll_events()
		{ }

		void begin_draw()
		{ }

		void end_draw(void* command_buffer)
		{ }
	};

#if defined USE_VULKAN_GRAPHICS && defined USE_SDL_WINDOWING
	class GUICore : public GenericGUICore<ImGUICore>
	{ };
#else
	class GUICore : public GenericGUICore<NoopGUICore>
	{ };
#endif

	extern GUICore global_gui_core;
}
