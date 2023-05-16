#include <graphics/api/vulkan/vk_framebuffer.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/swapchain.h>
#include <graphics/resource/image.h>
#include <window/window.h>

namespace Sunset
{
	void VulkanFramebuffer::initialize(class GraphicsContext* const gfx_context, void* render_pass_handle, const std::vector<RenderPassAttachmentInfo>& attachments)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		VkFramebufferCreateInfo fb_create_info = {};
		fb_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_create_info.pNext = nullptr;
		fb_create_info.attachmentCount = static_cast<int32_t>(attachments.size());
		fb_create_info.layers = 1;

		if (VkRenderPass* render_pass = static_cast<VkRenderPass*>(render_pass_handle))
		{
			fb_create_info.renderPass = *render_pass;
		}

		std::vector<VkImageView> fb_attachments;
		fb_attachments.reserve(fb_create_info.attachmentCount);

		uint32_t max_width{ 0 };
		uint32_t max_height{ 0 };
		for (const RenderPassAttachmentInfo& attachment : attachments)
		{
			Image* const image_attachment = CACHE_FETCH(Image, attachment.image);
			VkImageView attachment_image_view = static_cast<VkImageView>(image_attachment->get_image_view(attachment.array_index));
			fb_attachments.push_back(attachment_image_view);

			if (image_attachment->get_attachment_config().extent.x > max_width)
			{
				max_width = image_attachment->get_attachment_config().extent.x;
			}

			if (image_attachment->get_attachment_config().extent.y > max_height)
			{
				max_height = image_attachment->get_attachment_config().extent.y;
			}
		}

		if (max_width > 0 && max_height > 0)
		{
			extent.x = max_width;
			extent.y = max_height;
			
		}
		else
		{
			extent.x = context_state->window->get_extent().x;
			extent.y = context_state->window->get_extent().y;
		}

		fb_create_info.width = extent.x;
		fb_create_info.height = extent.y;

		fb_create_info.pAttachments = fb_attachments.data();

		VK_CHECK(vkCreateFramebuffer(context_state->get_device(), &fb_create_info, nullptr, &framebuffer));
	}

	void VulkanFramebuffer::destroy(class GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		vkDestroyFramebuffer(context_state->get_device(), framebuffer, nullptr);
	}
}
