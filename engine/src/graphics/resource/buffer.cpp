#include <graphics/resource/buffer.h>
#include <graphics/graphics_context.h>
#include <graphics/asset_pool.h>

namespace Sunset
{
	Sunset::Buffer* BufferFactory::create(class GraphicsContext* const gfx_context, size_t buffer_size, BufferType type)
	{
		Buffer* buffer = GlobalAssetPools<Buffer>::get()->allocate();
		buffer->initialize(gfx_context, buffer_size, type);
		gfx_context->add_resource_deletion_execution([buffer, gfx_context]()
		{
			buffer->destroy(gfx_context);
			GlobalAssetPools<Buffer>::get()->deallocate(buffer);
		});
		return buffer;
	}

	size_t BufferHelpers::pad_ubo_size(size_t ubo_size, size_t min_ubo_alignment)
	{
		size_t aligned_size = ubo_size;
		if (min_ubo_alignment > 0)
		{
			aligned_size = (aligned_size + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);
		}
		return aligned_size;
	}
}
