#include <graphics/resource/buffer.h>
#include <graphics/graphics_context.h>
#include <graphics/asset_pool.h>

namespace Sunset
{
	Sunset::BufferID BufferFactory::create(class GraphicsContext* const gfx_context, const BufferConfig& config, bool auto_delete)
	{
		bool b_added{ false };
		BufferID buffer_id = BufferCache::get()->fetch_or_add(config.name, gfx_context, b_added, auto_delete);
		if (b_added)
		{
			Buffer* buffer = CACHE_FETCH(Buffer, buffer_id);
			buffer->initialize(gfx_context, config);
		}
		return buffer_id;
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

	ScopedGPUBufferMapping::ScopedGPUBufferMapping(class GraphicsContext* const gfx_context, Buffer* buffer)
		: buffer(buffer), gfx_context(gfx_context)
	{
		mapped_memory = buffer->map_gpu(gfx_context);
	}

	ScopedGPUBufferMapping::~ScopedGPUBufferMapping()
	{
		buffer->unmap_gpu(gfx_context);
	}
}
