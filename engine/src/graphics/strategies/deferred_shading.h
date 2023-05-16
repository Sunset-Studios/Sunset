#pragma once

#include <minimal.h>
#include <graphics/render_graph.h>

namespace Sunset
{
	class DeferredShadingStrategy
	{
	public:
		void render(class GraphicsContext* gfx_context, RenderGraph& render_graph, class Swapchain* swapchain, bool b_offline = false);
	};
}
