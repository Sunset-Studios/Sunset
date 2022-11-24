#include <graphics/resource/image.h>
#include <graphics/asset_pool.h>

namespace Sunset
{
	Sunset::Image* ImageFactory::create(class GraphicsContext* const gfx_context, Format format, ImageType image_type, const glm::vec3& extent, ImageUsage usage)
	{
		Image* image = GlobalAssetPools<Image>::get()->allocate();
		image->initialize(gfx_context, format, image_type, extent, usage);
		return image;
	}
}
