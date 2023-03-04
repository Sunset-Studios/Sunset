#include <graphics/render_pass.h>

namespace Sunset
{
	Sunset::RenderPassID RenderPassFactory::create_default(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, const RenderPassConfig& config, bool b_auto_delete /*= false*/)
	{
		bool b_added{ false };
		RenderPassID rp_id = RenderPassCache::get()->fetch_or_add(config.name, gfx_context, b_added, b_auto_delete);
		if (b_added)
		{
			RenderPass* rp = CACHE_FETCH(RenderPass, rp_id);
			rp->initialize_default(gfx_context, swapchain, config);
		}
		return rp_id;
	}
}
