#include <graphics/api/vulkan/vk_swapchain.h>
#include <graphics/graphics_context.h>
#include <graphics/command_queue.h>
#include <window/window.h>

#include <VkBootstrap.h>

namespace Sunset
{
	void VulkanSwapchain::initialize(GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		vkb::SwapchainBuilder swapchain_builder{ context_state->get_gpu(), context_state->get_device(), context_state->surface };

		const glm::ivec2 window_extent = context_state->window->get_extent();
		vkb::Swapchain vkb_swapchain = swapchain_builder
			.use_default_format_selection()
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(window_extent.x, window_extent.y)
			.build()
			.value();

		data.swapchain = vkb_swapchain.swapchain;
		data.swapchain_images = vkb_swapchain.get_images().value();
		data.swapchain_image_views = vkb_swapchain.get_image_views().value();
		data.swapchain_image_format = vkb_swapchain.image_format;
	}

	void VulkanSwapchain::destroy(GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		vkDestroySwapchainKHR(context_state->get_device(), data.swapchain, nullptr);

		for (int i = 0; i < data.swapchain_image_views.size(); ++i)
		{
			vkDestroyImageView(context_state->get_device(), data.swapchain_image_views[i], nullptr);
		}
	}

	void VulkanSwapchain::request_next_image(class GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		VK_CHECK(vkAcquireNextImageKHR(context_state->get_device(), data.swapchain, 1000000000, context_state->sync_pool.get_semaphore(context_state->present_semaphore), nullptr, &data.current_image_index));
	}


	void VulkanSwapchain::present(class GraphicsContext* const gfx_context, class GraphicsCommandQueue* const command_queue)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		VulkanCommandQueueData* command_queue_data = static_cast<VulkanCommandQueueData*>(command_queue->get_data());

		VkSemaphore render_semaphore = context_state->sync_pool.get_semaphore(context_state->render_semaphore);

		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext = nullptr;
		present_info.pSwapchains = &data.swapchain;
		present_info.swapchainCount = 1;
		present_info.pWaitSemaphores = &render_semaphore;
		present_info.waitSemaphoreCount = 1;
		present_info.pImageIndices = &data.current_image_index;

		VK_CHECK(vkQueuePresentKHR(command_queue_data->graphics_queue, &present_info));
	}
}
