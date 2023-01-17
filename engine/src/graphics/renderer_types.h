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
}