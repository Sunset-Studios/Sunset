#include <graphics/api/vulkan/vk_render_pass.h>
#include <graphics/resource/swapchain.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/framebuffer.h>
#include <graphics/pipeline_state.h>
#include <window/window.h>
#include <input/input_provider.h>

namespace Sunset
{
	void VulkanRenderPass::initialize(GraphicsContext* const gfx_context, Swapchain* const swapchain)
	{
	}

	void VulkanRenderPass::initialize_default(class GraphicsContext* const gfx_context, class Swapchain* const swapchain)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		VulkanSwapchainData* swapchain_data = static_cast<VulkanSwapchainData*>(swapchain->get_data());

		VkAttachmentDescription color_attachment = {};
		color_attachment.format = swapchain_data->swapchain_image_format;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_ref = {};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;

		VkRenderPassCreateInfo render_pass_create_info = {};
		render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.attachmentCount = 1;
		render_pass_create_info.pAttachments = &color_attachment;
		render_pass_create_info.subpassCount = 1;
		render_pass_create_info.pSubpasses = &subpass;

		VK_CHECK(vkCreateRenderPass(context_state->get_device(), &render_pass_create_info, nullptr, &data.render_pass));

		create_default_output_framebuffers(gfx_context, swapchain);
	}

	void VulkanRenderPass::destroy(GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		vkDestroyRenderPass(context_state->get_device(), data.render_pass, nullptr);

		for (int i = 0; i < data.output_framebuffers.size(); ++i)
		{
			data.output_framebuffers[i]->destroy(gfx_context);
		}
	}

	void VulkanRenderPass::draw(class GraphicsContext* const gfx_context, void* command_buffer)
	{
	}

	void VulkanRenderPass::begin_pass(GraphicsContext* const gfx_context, Swapchain* const swapchain, void* command_buffer)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		VulkanSwapchainData* swapchain_data = static_cast<VulkanSwapchainData*>(swapchain->get_data());

		VkClearValue clear_value;
		const float flash = std::abs(std::sin(gfx_context->get_frame_number() / 120.0f));
		clear_value.color = { { 0.0f, 0.0f, flash, 1.0f } };

		VkRenderPassBeginInfo rp_begin_info = {};
		rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rp_begin_info.pNext = nullptr;
		rp_begin_info.renderPass = data.render_pass;
		rp_begin_info.renderArea.offset.x = 0;
		rp_begin_info.renderArea.offset.y = 0;
		rp_begin_info.renderArea.extent.width = context_state->window->get_extent().x;
		rp_begin_info.renderArea.extent.height = context_state->window->get_extent().y;
		rp_begin_info.framebuffer = static_cast<VkFramebuffer>(data.output_framebuffers[swapchain_data->current_image_index]->get_framebuffer_handle());

		rp_begin_info.clearValueCount = 1;
		rp_begin_info.pClearValues = &clear_value;

		vkCmdBeginRenderPass(static_cast<VkCommandBuffer>(command_buffer), &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	}


	void VulkanRenderPass::end_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer)
	{
		vkCmdEndRenderPass(static_cast<VkCommandBuffer>(command_buffer));
	}

	void VulkanRenderPass::create_default_output_framebuffers(GraphicsContext* const gfx_context, Swapchain* const swapchain)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		VulkanSwapchainData* swapchain_data = static_cast<VulkanSwapchainData*>(swapchain->get_data());

		const size_t swapchain_image_count = swapchain_data->swapchain_images.size();
		data.output_framebuffers = std::vector<Framebuffer*>(swapchain_image_count);

		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			data.output_framebuffers[i] = FramebufferFactory::create(gfx_context, swapchain, &data.render_pass, &swapchain_data->swapchain_image_views[i]);
		}
	}

}
