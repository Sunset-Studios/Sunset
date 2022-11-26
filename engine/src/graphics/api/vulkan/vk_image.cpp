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
		VkImageUsageFlagBits usage_flags(VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM);
		if (static_cast<int32_t>(flags & ImageFlags::Color) > 0)
		{
			usage_flags = static_cast<VkImageUsageFlagBits>(usage_flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
		}
		if (static_cast<int32_t>(flags & (ImageFlags::Depth | ImageFlags::Stencil)) > 0)
		{
			usage_flags = static_cast<VkImageUsageFlagBits>(usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		}
		if (static_cast<int32_t>(flags & ImageFlags::TransferSrc) > 0)
		{
			usage_flags = static_cast<VkImageUsageFlagBits>(usage_flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		}
		if (static_cast<int32_t>(flags & ImageFlags::TransferDst) > 0)
		{
			usage_flags = static_cast<VkImageUsageFlagBits>(usage_flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		}
		return usage_flags;
	}

	inline VkImageAspectFlags VK_FROM_SUNSET_IMAGE_USAGE_ASPECT_FLAGS(ImageFlags flags)
	{
		VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_NONE;
		if (static_cast<int32_t>(flags & ImageFlags::Color) > 0)
		{
			aspect_flags = static_cast<VkImageUsageFlagBits>(aspect_flags | VK_IMAGE_ASPECT_COLOR_BIT);
		}
		else if (static_cast<int32_t>(flags & ImageFlags::Depth) > 0)
		{
			aspect_flags = static_cast<VkImageUsageFlagBits>(aspect_flags | VK_IMAGE_ASPECT_DEPTH_BIT);
		}
		else if (static_cast<int32_t>(flags & ImageFlags::Stencil) > 0)
		{
			aspect_flags = static_cast<VkImageUsageFlagBits>(aspect_flags | VK_IMAGE_ASPECT_STENCIL_BIT);
		}
		return aspect_flags;
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
		img_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
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

		gfx_context->add_resource_deletion_execution([this, allocator, device = context_state->get_device()]()
		{
			vkDestroyImageView(device, image_view, nullptr);
			vmaDestroyImage(allocator, image, allocation);
		});
	}

	void VulkanImage::destroy(class GraphicsContext* const gfx_context)
	{

	}

	void VulkanImage::copy_from(class GraphicsContext* const gfx_context, void* data)
	{

	}

	void VulkanImage::bind(class GraphicsContext* const gfx_context, void* command_buffer)
	{

	}
}
