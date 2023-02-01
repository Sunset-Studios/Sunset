#include <graphics/render_pass.h>

namespace Sunset
{
	Sunset::RenderPassID RenderPassFactory::create_default_compute(class GraphicsContext* const gfx_context, const RenderPassConfig& config, bool b_auto_delete /*= false*/)
	{
		RenderPassID rp_id = RenderPassCache::get()->fetch_or_add(config.name, gfx_context);
		RenderPass* rp = CACHE_FETCH(RenderPass, rp_id);
		rp->initialize_default_compute(gfx_context, config);
		if (b_auto_delete)
		{
			gfx_context->add_resource_deletion_execution([rp_id, rp, gfx_context]()
			{
				RenderPassCache::get()->remove(rp_id);
				rp->destroy(gfx_context);
				GlobalAssetPools<RenderPass>::get()->deallocate(rp);
			});
		}
		return rp_id;
	}

	Sunset::RenderPassID RenderPassFactory::create_default_graphics(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, const RenderPassConfig& config, bool b_auto_delete /*= false*/)
	{
		RenderPassID rp_id = RenderPassCache::get()->fetch_or_add(config.name, gfx_context);
		RenderPass* rp = CACHE_FETCH(RenderPass, rp_id);
		rp->initialize_default_graphics(gfx_context, swapchain, config);
		if (b_auto_delete)
		{
			gfx_context->add_resource_deletion_execution([rp_id, rp, gfx_context]()
			{
				RenderPassCache::get()->remove(rp_id);
				rp->destroy(gfx_context);
				GlobalAssetPools<RenderPass>::get()->deallocate(rp);
			});
		}
		return rp_id;
	}
}
