
#include <minimal.h>
#include <graphics/render_graph.h>

namespace Sunset
{
	class ForwardShadingStrategy
	{
	public:
		bool render(class GraphicsContext* gfx_context, RenderGraph& render_graph, class Swapchain* swapchain, int32_t buffered_frame_number, bool b_offline = false);
	};
}
