#pragma once

#include <iostream>

#include <vulkan/vulkan.h>

inline void VK_CHECK(VkResult result)
{
	if (result)
	{
		std::cout << "Vulkan error detected: " << result << std::endl;
		abort();
	}
}
