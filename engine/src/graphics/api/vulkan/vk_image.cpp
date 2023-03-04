#include <graphics/resource/image.h>
#include <graphics/api/vulkan/vk_image.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>
#include <vk_mem_alloc.h>

#include <minimal.h>

namespace Sunset
{
	inline VkImageUsageFlags VK_FROM_SUNSET_IMAGE_USAGE(ImageFlags flags)
	{
		int32_t usage_flags{ 0 };
		if (static_cast<int32_t>(flags & ImageFlags::Color) > 0)
		{
			usage_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		if (static_cast<int32_t>(flags & (ImageFlags::DepthStencil)) > 0)
		{
			usage_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		if (static_cast<int32_t>(flags & ImageFlags::TransferSrc) > 0)
		{
			usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		if (static_cast<int32_t>(flags & ImageFlags::TransferDst) > 0)
		{
			usage_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		if (static_cast<int32_t>(flags & ImageFlags::Sampled) > 0)
		{
			usage_flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		return usage_flags > 0 ? static_cast<VkImageUsageFlags>(usage_flags) : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	inline VkImageAspectFlags VK_FROM_SUNSET_IMAGE_USAGE_ASPECT_FLAGS(ImageFlags flags)
	{
		int32_t aspect_flags{ 0 };
		if (static_cast<int32_t>(flags & ImageFlags::Color) > 0)
		{
			aspect_flags |= VK_IMAGE_ASPECT_COLOR_BIT;
		}
		else if (static_cast<int32_t>(flags & (ImageFlags::Depth | ImageFlags::DepthStencil)) > 0)
		{
			aspect_flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		else if (static_cast<int32_t>(flags & (ImageFlags::Depth | ImageFlags::DepthStencil)) > 0)
		{
			aspect_flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		return aspect_flags > 0 ? static_cast<VkImageAspectFlags>(aspect_flags) : VK_IMAGE_ASPECT_COLOR_BIT;
	}

	void VulkanImage::initialize(class GraphicsContext* const gfx_context, AttachmentConfig& config)
	{
		assert(gfx_context->get_buffer_allocator() != nullptr);

		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		assert(context_state != nullptr);

		VmaAllocator allocator = static_cast<VmaAllocator>(gfx_context->get_buffer_allocator()->get_handle());

		VkImageCreateInfo image_info = {};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.pNext = nullptr;
		image_info.imageType = VK_FROM_SUNSET_IMAGE_TYPE(config.flags);
		image_info.format = VK_FROM_SUNSET_FORMAT(config.format);
		image_info.extent = VkExtent3D{ static_cast<unsigned int>(config.extent.x), static_cast<unsigned int>(config.extent.y), static_cast<unsigned int>(glm::max(1.0f, config.extent.z)) };
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_info.usage = VK_FROM_SUNSET_IMAGE_USAGE(config.flags);

		VmaAllocationCreateInfo img_alloc_info = {};
		img_alloc_info.usage = VK_FROM_SUNSET_MEMORY_USAGE(config.usage_type);
		img_alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		vmaCreateImage(allocator, &image_info, &img_alloc_info, &image, &allocation, nullptr);

		VkImageViewCreateInfo image_view_info = {};
		image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_info.pNext = nullptr;
		image_view_info.viewType = VK_FROM_SUNSET_IMAGE_VIEW_TYPE(config.flags);
		image_view_info.image = image;
		image_view_info.format = VK_FROM_SUNSET_FORMAT(config.format);
		image_view_info.subresourceRange.baseMipLevel = 0;
		image_view_info.subresourceRange.levelCount = 1;
		image_view_info.subresourceRange.baseArrayLayer = 0;
		image_view_info.subresourceRange.layerCount = 1;
		image_view_info.subresourceRange.aspectMask = VK_FROM_SUNSET_IMAGE_USAGE_ASPECT_FLAGS(config.flags);

		VK_CHECK(vkCreateImageView(context_state->get_device(), &image_view_info, nullptr, &image_view));

		if ((config.flags & ImageFlags::Sampled) != ImageFlags::None)
		{
			VkSamplerCreateInfo sampler_create_info = {};
			sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			sampler_create_info.pNext = nullptr;
			sampler_create_info.magFilter = VK_FROM_SUNSET_IMAGE_FILTER(config.image_filter);
			sampler_create_info.minFilter = VK_FROM_SUNSET_IMAGE_FILTER(config.image_filter);
			sampler_create_info.addressModeU = VK_FROM_SUNSET_SAMPLER_ADDRESS_MODE(config.sampler_address_mode);
			sampler_create_info.addressModeV = VK_FROM_SUNSET_SAMPLER_ADDRESS_MODE(config.sampler_address_mode);
			sampler_create_info.addressModeW = VK_FROM_SUNSET_SAMPLER_ADDRESS_MODE(config.sampler_address_mode);

			vkCreateSampler(context_state->get_device(), &sampler_create_info, nullptr, &sampler);
		}
	}

	void VulkanImage::initialize(class GraphicsContext* const gfx_context, const AttachmentConfig& config, void* image_handle, void* image_view_handle)
	{
		VkImage existing_image = static_cast<VkImage>(image_handle);
		VkImageView existing_image_view = static_cast<VkImageView>(image_view_handle);
		assert(existing_image != nullptr && existing_image_view != nullptr);

		image = existing_image;
		image_view = existing_image_view;
		b_external_handling = true;
	}

	void VulkanImage::destroy(class GraphicsContext* const gfx_context)
	{
		if (b_external_handling)
		{
			return;
		}

		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		assert(context_state != nullptr);

		VmaAllocator allocator = static_cast<VmaAllocator>(gfx_context->get_buffer_allocator()->get_handle());

		vkDestroySampler(context_state->get_device(), sampler, nullptr);
		vkDestroyImageView(context_state->get_device(), image_view, nullptr);
		vmaDestroyImage(allocator, image, allocation);
	}

	void VulkanImage::copy_buffer(class GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, Buffer* buffer)
	{
		VkImageSubresourceRange range;
		range.aspectMask = VK_FROM_SUNSET_IMAGE_USAGE_ASPECT_FLAGS(config.flags);
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		VkImageMemoryBarrier image_barrier_to_transfer = {};
		image_barrier_to_transfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_barrier_to_transfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_barrier_to_transfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_barrier_to_transfer.image = image;
		image_barrier_to_transfer.subresourceRange = range;
		image_barrier_to_transfer.srcAccessMask = 0;
		image_barrier_to_transfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		VkCommandBuffer cmd = static_cast<VkCommandBuffer>(command_buffer);
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_barrier_to_transfer);

		VkBufferImageCopy buffer_image_copy = {};
		buffer_image_copy.bufferOffset = 0;
		buffer_image_copy.bufferRowLength = 0;
		buffer_image_copy.bufferImageHeight = 0;
		buffer_image_copy.imageSubresource.aspectMask = VK_FROM_SUNSET_IMAGE_USAGE_ASPECT_FLAGS(config.flags);
		buffer_image_copy.imageSubresource.mipLevel = 0;
		buffer_image_copy.imageSubresource.baseArrayLayer = 0;
		buffer_image_copy.imageSubresource.layerCount = 1;
		buffer_image_copy.imageExtent = VkExtent3D{ static_cast<unsigned int>(config.extent.x), static_cast<unsigned int>(config.extent.y), static_cast<unsigned int>(glm::max(1.0f, config.extent.z)) };

		VkBuffer other_buffer = static_cast<VkBuffer>(buffer->get());
		vkCmdCopyBufferToImage(cmd, other_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);

		VkImageMemoryBarrier image_barrier_to_readable = image_barrier_to_transfer;
		image_barrier_to_readable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_barrier_to_readable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_barrier_to_readable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		image_barrier_to_readable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_barrier_to_readable);
	}

	void VulkanImage::bind(class GraphicsContext* const gfx_context, void* command_buffer)
	{

	}
}
