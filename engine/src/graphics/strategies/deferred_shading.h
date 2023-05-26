#pragma once

#include <minimal.h>
#include <utility/pattern/singleton.h>
#include <graphics/render_graph.h>

namespace Sunset
{
	class DeferredShadingStrategy
	{
	public:
		void render(class GraphicsContext* gfx_context, RenderGraph& render_graph, class Swapchain* swapchain, bool b_offline = false);
	};

	class DeferredShadingPersistentStorage : public Singleton<DeferredShadingPersistentStorage>
	{
		friend class Singleton;
		friend class DeferredShadingStrategy;

	public:
		void initialize();

	protected:
		bool b_initialized{ false };
		BufferID ssao_data_buffer[MAX_BUFFERED_FRAMES];
	};
}
