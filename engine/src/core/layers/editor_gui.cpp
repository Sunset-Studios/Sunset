#include <core/layers/editor_gui.h>
#include <graphics/renderer.h>
#include <utility/gui/gui_core.h>

namespace Sunset
{
	void EditorGui::initialize()
	{
	}

	void EditorGui::destroy()
	{
	}

	void EditorGui::update(double delta_time)
	{
		global_gui_core.poll_events();
	}
}
