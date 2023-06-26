#include <graphics/api/vulkan/vk_swapchain.h>
#include <graphics/graphics_context.h>
#include <graphics/command_queue.h>
#include <graphics/resource/image.h>
#include <window/window.h>

#include <VkBootstrap.h>

namespace Sunset
{
	void VulkanSwapchain::initialize(GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		vkb::SwapchainBuilder swapchain_builder{ context_state->get_gpu(), context_state->get_device(), context_state->surface };

		const glm::ivec2 window_extent = context_state->surface_resolution;
		vkb::Swapchain vkb_swapchain = swapchain_builder
			.use_default_format_selection()
			.set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
			.set_desired_extent(window_extent.x, window_extent.y)
			.build()
			.value();

		data.swapchain = vkb_swapchain.swapchain;
		data.swapchain_image_format = vkb_swapchain.image_format;

		std::vector<VkImage> swapchain_images = vkb_swapchain.get_images().value();
		std::vector<VkImageView> swapchain_image_views = vkb_swapchain.get_image_views().value();
		for (int i = 0; i < swapchain_images.size(); ++i)
		{
			data.swapchain_images.push_back(
				ImageFactory::create(
					gfx_context,
					{ .name = std::string_view("swapchain_" + i) },
					swapchain_images[i],
					swapchain_image_views[i]
				)
			);
		}
	}

	void VulkanSwapchain::destroy(GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		vkDestroySwapchainKHR(context_state->get_device(), data.swapchain, nullptr);

		for (int i = 0; i < data.swapchain_images.size(); ++i)
		{
			Image* const image = CACHE_FETCH(Image, data.swapchain_images[i]);
			vkDestroyImageView(context_state->get_device(), static_cast<VkImageView>(image->get_image_view()), nullptr);
			CACHE_DELETE(Image, data.swapchain_images[i], gfx_context);
		}
	}

	void VulkanSwapchain::request_next_image(GraphicsContext* const gfx_context, int32_t buffered_frame)
	{
		ZoneScopedN("VulkanSwapchain::request_next_image");

		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		VK_CHECK(vkAcquireNextImageKHR(context_state->get_device(), data.swapchain, 1000000000, context_state->sync_pool.get_semaphore(context_state->frame_sync_primitives[buffered_frame].present_semaphore), nullptr, &data.current_image_index[buffered_frame]));
	}

	void VulkanSwapchain::present(GraphicsContext* const gfx_context, DeviceQueueType queue_type, int32_t buffered_frame)
	{
		ZoneScopedN("VulkanSwapchain::present");

		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		VulkanCommandQueueData* command_queue_data = static_cast<VulkanCommandQueueData*>(gfx_context->get_command_queue(queue_type)->get_data());

		VkSemaphore render_semaphore = context_state->sync_pool.get_semaphore(context_state->frame_sync_primitives[buffered_frame].render_semaphore);

		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext = nullptr;
		present_info.pSwapchains = &data.swapchain;
		present_info.swapchainCount = 1;
		present_info.pWaitSemaphores = &render_semaphore;
		present_info.waitSemaphoreCount = 1;
		present_info.pImageIndices = &data.current_image_index[buffered_frame];

		VK_CHECK(vkQueuePresentKHR(command_queue_data->graphics_queue, &present_info));
	}
}
