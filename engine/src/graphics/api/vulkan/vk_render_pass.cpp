#include <graphics/api/vulkan/vk_render_pass.h>
#include <graphics/resource/swapchain.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/framebuffer.h>
#include <graphics/pipeline_state.h>
#include <graphics/resource/image.h>
#include <window/window.h>
#include <input/input_provider.h>

namespace Sunset
{
	void VulkanRenderPass::initialize_default(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, const RenderPassConfig& config)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		std::vector<VkAttachmentDescription> vk_attachments;
		std::vector<VkSubpassDependency> vk_dependencies;
		std::vector<VkAttachmentReference> vk_color_attachment_references;
		std::vector<VkAttachmentReference> vk_depth_stencil_attachment_references;

		{
			if (config.b_is_present_pass)
			{
				VulkanSwapchainData* swapchain_data = static_cast<VulkanSwapchainData*>(swapchain->get_data());

				VkAttachmentDescription swapchain_image_attachment = {};
				swapchain_image_attachment.format = swapchain_data->swapchain_image_format;
				swapchain_image_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
				swapchain_image_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				swapchain_image_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				swapchain_image_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				swapchain_image_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				swapchain_image_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				swapchain_image_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

				VkAttachmentReference swapchain_image_attachment_ref = {};
				swapchain_image_attachment_ref.attachment = 0;
				swapchain_image_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				VkSubpassDependency swapchain_image_dependency = {};
				swapchain_image_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				swapchain_image_dependency.dstSubpass = 0;
				swapchain_image_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				swapchain_image_dependency.srcAccessMask = 0;
				swapchain_image_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				swapchain_image_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				vk_attachments.push_back(swapchain_image_attachment);
				vk_dependencies.push_back(swapchain_image_dependency);
			}

			uint32_t attachment_index = static_cast<uint32_t>(config.b_is_present_pass);
			for (ImageID image_attachment_id : config.attachments)
			{
				Image* const image_attachment = CACHE_FETCH(Image, image_attachment_id);
				AttachmentConfig& image_attachment_config = image_attachment->get_attachment_config();

				VkAttachmentDescription attachment = {};
				attachment.format = VK_FROM_SUNSET_FORMAT(image_attachment_config.format);
				attachment.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment.loadOp = image_attachment_config.attachment_clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				attachment.storeOp = image_attachment_config.has_store_op ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment.stencilLoadOp = image_attachment_config.attachment_stencil_clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				if ((image_attachment_config.flags & ImageFlags::DepthStencil) != ImageFlags::None)
				{
					attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

					VkAttachmentReference& attachment_ref = vk_depth_stencil_attachment_references.emplace_back(VkAttachmentReference());
					attachment_ref.attachment = attachment_index;
					attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}
			
				if ((image_attachment_config.flags & ImageFlags::Color) != ImageFlags::None)
				{
					VkAttachmentReference& attachment_ref = vk_color_attachment_references.emplace_back(VkAttachmentReference());
					attachment_ref.attachment = attachment_index;
					attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}

				if ((image_attachment_config.flags & ImageFlags::Present) != ImageFlags::None)
				{
					attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				}

				VkSubpassDependency dependency = {};
				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0;
				dependency.srcAccessMask = 0;

				if ((image_attachment_config.flags & ImageFlags::DepthStencil) != ImageFlags::None)
				{
					dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				}
				else if ((image_attachment_config.flags & ImageFlags::Color) != ImageFlags::None)
				{
					dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				}

				vk_attachments.push_back(attachment);
				vk_dependencies.push_back(dependency);

				++attachment_index;
			}
		}

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = vk_color_attachment_references.size();
		subpass.pColorAttachments = vk_color_attachment_references.data();
		if (!vk_depth_stencil_attachment_references.empty())
		{
			subpass.pDepthStencilAttachment = vk_depth_stencil_attachment_references.data();
		}

		VkRenderPassCreateInfo render_pass_create_info = {};
		render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.attachmentCount = static_cast<uint32_t>(vk_attachments.size());
		render_pass_create_info.pAttachments = vk_attachments.data();
		render_pass_create_info.dependencyCount = static_cast<uint32_t>(vk_dependencies.size());
		render_pass_create_info.pDependencies = vk_dependencies.data();
		render_pass_create_info.subpassCount = 1;
		render_pass_create_info.pSubpasses = &subpass;

		VK_CHECK(vkCreateRenderPass(context_state->get_device(), &render_pass_create_info, nullptr, &data.render_pass));

		create_default_output_framebuffers(gfx_context, swapchain, config, config.attachments);
	}

	void VulkanRenderPass::destroy(GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		vkDestroyRenderPass(context_state->get_device(), data.render_pass, nullptr);

		for (int i = 0; i < data.output_framebuffers.size(); ++i)
		{
			CACHE_FETCH(Framebuffer, data.output_framebuffers[i])->destroy(gfx_context);
		}
	}

	void VulkanRenderPass::begin_pass(GraphicsContext* const gfx_context, uint32_t frambuffer_index, void* command_buffer)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		VkRenderPassBeginInfo rp_begin_info = {};
		rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rp_begin_info.pNext = nullptr;
		rp_begin_info.renderPass = data.render_pass;
		rp_begin_info.renderArea.offset.x = 0;
		rp_begin_info.renderArea.offset.y = 0;
		rp_begin_info.renderArea.extent.width = context_state->window->get_extent().x;
		rp_begin_info.renderArea.extent.height = context_state->window->get_extent().y;

		rp_begin_info.framebuffer = static_cast<VkFramebuffer>(CACHE_FETCH(Framebuffer, data.output_framebuffers[frambuffer_index])->get_framebuffer_handle());

		VkClearValue clear_value;
		const float flash = std::abs(std::sin(gfx_context->get_frame_number() / 120.0f));
		clear_value.color = { { 0.0f, 0.0f, flash, 1.0f } };

		VkClearValue depth_clear_value;
		depth_clear_value.depthStencil.depth = 1.0f;

		std::vector<VkClearValue> clear_values{ clear_value, depth_clear_value };

		rp_begin_info.clearValueCount = clear_values.size();
		rp_begin_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(static_cast<VkCommandBuffer>(command_buffer), &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	}


	void VulkanRenderPass::end_pass(class GraphicsContext* const gfx_context, void* command_buffer)
	{
		vkCmdEndRenderPass(static_cast<VkCommandBuffer>(command_buffer));
	}

	void VulkanRenderPass::create_default_output_framebuffers(GraphicsContext* const gfx_context, Swapchain* const swapchain, const RenderPassConfig& config, const std::vector<ImageID>& attachments)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		if (config.b_is_present_pass)
		{
			VulkanSwapchainData* swapchain_data = static_cast<VulkanSwapchainData*>(swapchain->get_data());

			const size_t swapchain_image_count = swapchain_data->swapchain_images.size();
			data.output_framebuffers = std::vector<FramebufferID>(swapchain_image_count);

			for (size_t i = 0; i < swapchain_image_count; ++i)
			{
				std::vector<ImageID> all_attachments = { swapchain_data->swapchain_images[i] };
				all_attachments.insert(all_attachments.end(), attachments.begin(), attachments.end());
				data.output_framebuffers[i] = FramebufferFactory::create(gfx_context, &data.render_pass, all_attachments);
			}
		}
		else
		{
			data.output_framebuffers.push_back(
				FramebufferFactory::create(gfx_context, &data.render_pass, attachments)
			);
		}
	}

}
