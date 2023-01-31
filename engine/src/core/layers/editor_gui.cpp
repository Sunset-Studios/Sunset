#include <core/layers/editor_gui.h>
#include <graphics/renderer.h>
#include <utility/gui/gui_core.h>

namespace Sunset
{
	void EditorGui::initialize()
	{
		GraphicsContext* const gfx_context = Renderer::get()->context();
		global_gui_core.initialize(gfx_context, gfx_context->get_window());
	}

	void EditorGui::destroy()
	{
	}

	void EditorGui::update(double delta_time)
	{
		global_gui_core.poll_events();
		global_gui_core.new_frame(Renderer::get()->context()->get_window());
	}
}
