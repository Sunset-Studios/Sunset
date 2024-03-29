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

		if (VkRenderPass* render_pass = static_cast<VkRenderPass*>(render_pass_handle))
		{
			fb_create_info.renderPass = *render_pass;
		}

		std::vector<VkImageView> fb_attachments;
		fb_attachments.reserve(fb_create_info.attachmentCount);

		uint32_t max_width{ 0 };
		uint32_t max_height{ 0 };
		uint32_t max_layer_count{ 1 };
		for (const RenderPassAttachmentInfo& attachment : attachments)
		{
			Image* const image_attachment = CACHE_FETCH(Image, attachment.image);
			VkImageView attachment_image_view = static_cast<VkImageView>(image_attachment->get_image_view(attachment.image_view_index));
			fb_attachments.push_back(attachment_image_view);

			const AttachmentConfig& attachment_config = image_attachment->get_attachment_config();

			const uint32_t mip_index = attachment_config.split_array_layer_views ? attachment.image_view_index / attachment_config.array_count : attachment.image_view_index;
			if (uint32_t width = glm::clamp(static_cast<uint32_t>(attachment_config.extent.x) >> mip_index, 0u, static_cast<uint32_t>(attachment_config.extent.x)); width > max_width)
			{
				max_width = width; 
			}

			if (uint32_t height = glm::clamp(static_cast<uint32_t>(attachment_config.extent.y) >> mip_index, 0u, static_cast<uint32_t>(attachment_config.extent.y)); height > max_height)
			{
				max_height = height;
			}

			if (attachment.b_image_view_considers_layer_split)
			{
				max_layer_count = 1;
			}
			else if (uint32_t layer_count = attachment_config.array_count; layer_count > max_layer_count)
			{
				max_layer_count = layer_count;
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

		fb_create_info.layers = max_layer_count;
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
