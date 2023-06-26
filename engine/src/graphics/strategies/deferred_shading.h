#pragma once

#include <minimal.h>
#include <utility/pattern/singleton.h>
#include <graphics/render_graph.h>

namespace Sunset
{
	class DeferredShadingStrategy
	{
	public:
		bool render(class GraphicsContext* gfx_context, RenderGraph& render_graph, class Swapchain* swapchain, int32_t buffered_frame_number, bool b_offline = false);
	};

	class DeferredShadingPersistentStorage : public Singleton<DeferredShadingPersistentStorage>
	{
		friend class Singleton;
		friend class DeferredShadingStrategy;

	public:
		void initialize();

	protected:
		std::atomic_bool b_initialized{ false };
		BufferID ssao_data_buffer[MAX_BUFFERED_FRAMES];
	};
}
