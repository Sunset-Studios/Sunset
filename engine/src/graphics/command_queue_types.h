#pragma once

#include <minimal.h>

namespace Sunset
{
	enum class DeviceQueueType : uint8_t
	{
		None = 0,
		Graphics = 1,
		Compute = 2,
		Transfer = 3,
		Num = Transfer
	};
}