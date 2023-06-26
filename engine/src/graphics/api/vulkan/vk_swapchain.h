#pragma once

#include <vector>

#include <vk_types.h>

#include <graphics/command_queue_types.h>

namespace Sunset
{
	struct VulkanSwapchainData
	{
		public:
			VkSwapchainKHR swapchain;
			VkFormat swapchain_image_format;
			std::vector<ImageID> swapchain_images;
			uint32_t current_image_index[MAX_BUFFERED_FRAMES];
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

			uint32_t get_current_image_index(int32_t buffered_frame)
			{
				return data.current_image_index[buffered_frame];
			}

			void request_next_image(class GraphicsContext* const gfx_context, int32_t buffered_frame);
			void present(class GraphicsContext* const gfx_context, DeviceQueueType queue_type, int32_t buffered_frame);

		protected:
			VulkanSwapchainData data;
	};
}
