#pragma once

#include <vector>

#include <vk_types.h>
#include <vk_initializers.h>

namespace Sunset
{
	struct VulkanSwapchainData
	{
		public:
			VkSwapchainKHR swapchain;
			VkFormat swapchain_image_format;
			std::vector<VkImage> swapchain_images;
			std::vector<VkImageView> swapchain_image_views;
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

			void request_next_image(class GraphicsContext* const gfx_context);
			void present(class GraphicsContext* const gfx_context, DeviceQueueType queue_type);

		protected:
			VulkanSwapchainData data;
	};
}
