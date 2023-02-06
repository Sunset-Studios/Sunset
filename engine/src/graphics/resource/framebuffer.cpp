#include "graphics/resource/framebuffer.h"

namespace Sunset
{
	Sunset::FramebufferID FramebufferFactory::create(class GraphicsContext* const gfx_context, void* render_pass_handle /*= nullptr*/, const std::vector<ImageID>& attachments /*= {}*/, bool b_auto_delete /*= false*/)
	{
		bool b_added{ false };
		FramebufferID fb_id = FramebufferCache::get()->fetch_or_add(hash_attachments_list(attachments), gfx_context, b_added, b_auto_delete);
		if (b_added)
		{
			Framebuffer* fb = CACHE_FETCH(Framebuffer, fb_id);
			fb->initialize(gfx_context, render_pass_handle, attachments);
		}
		return fb_id;
	}
}
