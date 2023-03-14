#pragma once

#include <minimal.h>
#include <utility/strings.h>

namespace Sunset
{
	struct BufferConfig
	{
		Identity name;
		size_t buffer_size;
		BufferType type{ BufferType::StorageBuffer };
		MemoryUsageType memory_usage{ MemoryUsageType::CPUToGPU };
		bool b_is_bindless{ false };
	};
}