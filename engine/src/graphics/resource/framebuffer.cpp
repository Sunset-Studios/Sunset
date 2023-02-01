#include "graphics/resource/framebuffer.h"

namespace Sunset
{
	Sunset::FramebufferID FramebufferFactory::create(class GraphicsContext* const gfx_context, void* render_pass_handle /*= nullptr*/, const std::initializer_list<ImageID>& attachments /*= {}*/, bool b_auto_delete /*= false*/)
	{
		FramebufferID fb_id = FramebufferCache::get()->fetch_or_add(hash_attachments_list(attachments), gfx_context);
		Framebuffer* fb = CACHE_FETCH(Framebuffer, fb_id);
		fb->initialize(gfx_context, render_pass_handle, attachments);
		if (b_auto_delete)
		{
			gfx_context->add_resource_deletion_execution([fb_id, fb, gfx_context]()
				{
					FramebufferCache::get()->remove(fb_id);
					fb->destroy(gfx_context);
					GlobalAssetPools<Framebuffer>::get()->deallocate(fb);
				});
		}
		return fb_id;
	}
}
