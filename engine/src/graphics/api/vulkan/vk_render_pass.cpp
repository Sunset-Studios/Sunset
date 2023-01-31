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
	void VulkanRenderPass::initialize_default_compute(GraphicsContext* const gfx_context, Swapchain* const swapchain)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		VulkanSwapchainData* swapchain_data = static_cast<VulkanSwapchainData*>(swapchain->get_data());

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

		VkRenderPassCreateInfo render_pass_create_info = {};
		render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.subpassCount = 1;
		render_pass_create_info.pSubpasses = &subpass;

		VK_CHECK(vkCreateRenderPass(context_state->get_device(), &render_pass_create_info, nullptr, &data.render_pass));
	}

	void VulkanRenderPass::initialize_default_graphics(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, const RenderPassConfig& config)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		VulkanSwapchainData* swapchain_data = static_cast<VulkanSwapchainData*>(swapchain->get_data());

		// Default color attachment
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

		VkSubpassDependency color_dependency = {};
		color_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		color_dependency.dstSubpass = 0;
		color_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		color_dependency.srcAccessMask = 0;
		color_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		color_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::vector<VkAttachmentDescription> vk_attachments = { color_attachment };
		std::vector<VkSubpassDependency> vk_dependencies = { color_dependency };

		std::optional<VkAttachmentReference> main_depth_attachment_ref;

		uint32_t AttachmentIndex = 1;
		for (ImageID image_attachment_id : config.attachments)
		{
			Image* const image_attachment = ImageCache::get()->fetch(image_attachment_id);

			// Depth attachment
			VkAttachmentDescription attachment = {};
			attachment.format = VK_FROM_SUNSET_FORMAT(image_attachment->get_attachment_config().format);
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp = image_attachment->get_attachment_config().attachment_clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			attachment.storeOp = image_attachment->get_attachment_config().has_store_op ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.stencilLoadOp = image_attachment->get_attachment_config().attachment_stencil_clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = image_attachment->get_attachment_config().is_main_depth_attachment ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // May also want to check the format to see if it is a depth based attachment

			if (image_attachment->get_attachment_config().is_main_depth_attachment)
			{
				main_depth_attachment_ref.emplace(VkAttachmentReference());
				(*main_depth_attachment_ref).attachment = AttachmentIndex;
				(*main_depth_attachment_ref).layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			vk_attachments.push_back(attachment);
			vk_dependencies.push_back(dependency);

			++AttachmentIndex;
		}

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
		if (main_depth_attachment_ref.has_value())
		{
			subpass.pDepthStencilAttachment = &(*main_depth_attachment_ref);
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

		create_default_output_framebuffers(gfx_context, swapchain, config.attachments);
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

	void VulkanRenderPass::submit(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer)
	{
	}

	void VulkanRenderPass::begin_pass(GraphicsContext* const gfx_context, Swapchain* const swapchain, void* command_buffer)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		VulkanSwapchainData* swapchain_data = static_cast<VulkanSwapchainData*>(swapchain->get_data());

		VkRenderPassBeginInfo rp_begin_info = {};
		rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rp_begin_info.pNext = nullptr;
		rp_begin_info.renderPass = data.render_pass;
		rp_begin_info.renderArea.offset.x = 0;
		rp_begin_info.renderArea.offset.y = 0;
		rp_begin_info.renderArea.extent.width = context_state->window->get_extent().x;
		rp_begin_info.renderArea.extent.height = context_state->window->get_extent().y;

		// A null framebuffer probably means we're using this pass for compute work
		if (data.output_framebuffers[swapchain_data->current_image_index] != nullptr)
		{
			rp_begin_info.framebuffer = static_cast<VkFramebuffer>(data.output_framebuffers[swapchain_data->current_image_index]->get_framebuffer_handle());

			VkClearValue clear_value;
			const float flash = std::abs(std::sin(gfx_context->get_frame_number() / 120.0f));
			clear_value.color = { { 0.0f, 0.0f, flash, 1.0f } };

			VkClearValue depth_clear_value;
			depth_clear_value.depthStencil.depth = 1.0f;

			VkClearValue clear_values[] = { clear_value, depth_clear_value };

			rp_begin_info.clearValueCount = 2;
			rp_begin_info.pClearValues = &clear_values[0];
		}

		vkCmdBeginRenderPass(static_cast<VkCommandBuffer>(command_buffer), &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	}


	void VulkanRenderPass::end_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* command_buffer)
	{
		vkCmdEndRenderPass(static_cast<VkCommandBuffer>(command_buffer));
	}

	void VulkanRenderPass::create_default_output_framebuffers(GraphicsContext* const gfx_context, Swapchain* const swapchain, const std::initializer_list<ImageID>& attachments)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		VulkanSwapchainData* swapchain_data = static_cast<VulkanSwapchainData*>(swapchain->get_data());

		const size_t swapchain_image_count = swapchain_data->swapchain_images.size();
		data.output_framebuffers = std::vector<Framebuffer*>(swapchain_image_count);

		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			data.output_framebuffers[i] = FramebufferFactory::create(gfx_context, swapchain, &data.render_pass, &swapchain_data->swapchain_image_views[i], attachments);
		}
	}

}
