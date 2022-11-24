#include <graphics/api/vulkan/vk_image.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>
#include <vk_mem_alloc.h>

#include <minimal.h>

namespace Sunset
{
	inline VkImageUsageFlags VK_FROM_SUNSET_IMAGE_USAGE(ImageUsage usage)
	{
		switch (usage)
		{
			case ImageUsage::Color:
				return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			case ImageUsage::DepthStencil:
				return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			case ImageUsage::TransferSrc:
				return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			case ImageUsage::TransferDst:
				return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			default:
				return VK_IMAGE_USAGE_STORAGE_BIT;
		}
	}

	void VulkanImage::initialize(class GraphicsContext* const gfx_context, Format format, ImageType image_type, const glm::vec3& extent, ImageUsage usage)
	{
		assert(gfx_context->get_buffer_allocator() != nullptr);

		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		assert(context_state != nullptr);

		VmaAllocator allocator = static_cast<VmaAllocator>(gfx_context->get_buffer_allocator()->get_handle());

		VkImageCreateInfo image_info = {};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.pNext = nullptr;
		image_info.imageType = VK_FROM_SUNSET_IMAGE_TYPE(image_type);
		image_info.format = VK_FROM_SUNSET_FORMAT(format);
		image_info.extent = VkExtent3D{ static_cast<unsigned int>(extent.x), static_cast<unsigned int>(extent.y), static_cast<unsigned int>(extent.z) };
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_info.usage = VK_FROM_SUNSET_IMAGE_USAGE(usage);

		VmaAllocationCreateInfo img_alloc_info = {};
		img_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		img_alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		vmaCreateImage(allocator, &image_info, &img_alloc_info, &image, &allocation, nullptr);

		VkImageViewCreateInfo image_view_info = {};
		image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_info.pNext = nullptr;
		image_view_info.viewType = VK_FROM_SUNSET_IMAGE_VIEW_TYPE(image_type);
		image_view_info.image = image;
		image_view_info.format = VK_FROM_SUNSET_FORMAT(format);
		image_view_info.subresourceRange.baseMipLevel = 0;
		image_view_info.subresourceRange.levelCount = 1;
		image_view_info.subresourceRange.baseArrayLayer = 0;
		image_view_info.subresourceRange.layerCount = 1;
		image_view_info.subresourceRange.aspectMask = aspectFlags;

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
