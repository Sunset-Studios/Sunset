#include <graphics/resource/image.h>
#include <graphics/asset_pool.h>

namespace Sunset
{
	Sunset::Image* ImageFactory::create(class GraphicsContext* const gfx_context, const AttachmentConfig& config)
	{
		Image* image = GlobalAssetPools<Image>::get()->allocate();
		image->initialize(gfx_context, config);
		return image;
	}
}
