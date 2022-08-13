#include <graphics/api/vulkan/vk_framebuffer.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/swapchain.h>
#include <window/window.h>

namespace Sunset
{
	void VulkanFramebuffer::initialize(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* render_pass_handle, void* attachments_handle)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		VulkanSwapchainData* swapchain_data = static_cast<VulkanSwapchainData*>(swapchain->get_data());

		VkFramebufferCreateInfo fb_create_info = {};
		fb_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_create_info.pNext = nullptr;
		fb_create_info.attachmentCount = 1;
		fb_create_info.width = context_state->window->get_extent().x;
		fb_create_info.height = context_state->window->get_extent().y;
		fb_create_info.layers = 1;

		if (VkRenderPass* render_pass = static_cast<VkRenderPass*>(render_pass_handle))
		{
			fb_create_info.renderPass = *render_pass;
		}

		if (VkImageView* image_view = static_cast<VkImageView*>(attachments_handle))
		{
			fb_create_info.pAttachments = image_view;
		}

		VK_CHECK(vkCreateFramebuffer(context_state->get_device(), &fb_create_info, nullptr, &framebuffer));
	}

	void VulkanFramebuffer::destroy(class GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		vkDestroyFramebuffer(context_state->get_device(), framebuffer, nullptr);
	}
}
