#include <core/layers/editor_gui.h>
#include <graphics/renderer.h>
#include <utility/gui/gui_core.h>

namespace Sunset
{
	void EditorGui::initialize()
	{
		global_gui_core.initialize(Renderer::get()->context(), Renderer::get()->window());
	}

	void EditorGui::destroy()
	{
	}

	void EditorGui::update(double delta_time)
	{
		global_gui_core.poll_events();
		global_gui_core.new_frame(Renderer::get()->window());
	}
}
