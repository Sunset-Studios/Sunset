#pragma once

#include <vector>

#include <vk_types.h>
#include <vk_initializers.h>

#include <graphics/command_queue_types.h>

namespace Sunset
{
	struct VulkanSwapchainData
	{
		public:
			VkSwapchainKHR swapchain;
			VkFormat swapchain_image_format;
			std::vector<ImageID> swapchain_images;
			uint32_t current_image_index;
	};

	class VulkanSwapchain
	{
		public:
			VulkanSwapchain() = default;
			~VulkanSwapchain() = default;

		public:
			void initialize(class GraphicsContext* const gfx_context);
			void destroy(class GraphicsContext* const gfx_context);

			void* get_data()
			{
				return &data;
			}

			Format get_format()
			{
				return SUNSET_FROM_VK_FORMAT(data.swapchain_image_format);
			}

			uint32_t get_current_image_index()
			{
				return data.current_image_index;
			}

			void request_next_image(class GraphicsContext* const gfx_context);
			void present(class GraphicsContext* const gfx_context, DeviceQueueType queue_type);

		protected:
			VulkanSwapchainData data;
	};
}
