#pragma once

#include <minimal.h>
#include <graphics/resource_types.h>

namespace Sunset
{
	struct IndirectDrawBatch
	{
		MaterialID material;
		ResourceStateID resource_state;
		uint32_t first;
		uint32_t count;
	};

	struct GPUInstanceIndirectBufferData
	{
		class Buffer* cleared_draw_indirect_buffer;
		class Buffer* draw_indirect_buffer;
		class Buffer* object_instance_buffer;
		class Buffer* compacted_object_instance_buffer;
		bool b_needs_refresh{ false };
	};

	struct GPUObjectInstance
	{
		uint32_t object_id;
		uint32_t batch_id;
	};
}