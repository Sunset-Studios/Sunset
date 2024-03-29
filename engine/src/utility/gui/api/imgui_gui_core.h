#pragma once

#include <minimal.h>

namespace Sunset
{
	class ImGUICore
	{
	public:
		void initialize(class GraphicsContext* gfx_context, class Window* const window, RenderPassID render_pass);
		void new_frame(class Window* const window);
		void poll_events();
		void begin_draw();
		void end_draw(void* command_buffer);

	protected:
		bool b_initialized{ false };
	};
}
