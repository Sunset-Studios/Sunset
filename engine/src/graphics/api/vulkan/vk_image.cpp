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
		if (static_cast<int32_t>(flags & ImageFlags::Storage) > 0)
		{
			usage_flags |= VK_IMAGE_USAGE_STORAGE_BIT;
		}
		return usage_flags > 0 ? static_cast<VkImageUsageFlags>(usage_flags) : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	inline VkImageCreateFlagBits VK_FROM_SUNSET_IMAGE_FLAGS(ImageFlags flags)
	{
		int32_t usage_flags{ 0 };
		if (static_cast<int32_t>(flags & ImageFlags::Cube) > 0)
		{
			usage_flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}
		return static_cast<VkImageCreateFlagBits>(usage_flags);
	}

	inline VkImageTiling VK_FROM_SUNSET_IMAGE_TILING(ImageFlags flags)
	{
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		if ((flags & ImageFlags::LinearTiling) != ImageFlags::None)
		{
			tiling = VK_IMAGE_TILING_LINEAR;
		}
		return tiling;
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
		image_info.mipLevels = config.mip_count;
		image_info.arrayLayers = config.array_count;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.tiling = VK_FROM_SUNSET_IMAGE_TILING(config.flags);
		image_info.usage = VK_FROM_SUNSET_IMAGE_USAGE(config.flags);
		image_info.flags = VK_FROM_SUNSET_IMAGE_FLAGS(config.flags);

		VmaAllocationCreateInfo img_alloc_info = {};
		img_alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_FROM_SUNSET_MEMORY_FLAGS(config.usage_type));

		vmaCreateImage(allocator, &image_info, &img_alloc_info, &image, &allocation, nullptr);

		image_views.reserve(config.mip_count);
		for (uint32_t i = 0; i < config.mip_count; ++i)
		{
			VkImageViewCreateInfo image_view_info = {};
			image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_info.pNext = nullptr;
			image_view_info.viewType = VK_FROM_SUNSET_IMAGE_VIEW_TYPE(config.flags);
			image_view_info.image = image;
			image_view_info.format = VK_FROM_SUNSET_FORMAT(config.format);
			image_view_info.subresourceRange.baseMipLevel = i;
			image_view_info.subresourceRange.levelCount = 1;
			image_view_info.subresourceRange.baseArrayLayer = 0;
			image_view_info.subresourceRange.layerCount = config.array_count;
			image_view_info.subresourceRange.aspectMask = VK_FROM_SUNSET_IMAGE_USAGE_ASPECT_FLAGS(config.flags);

			VkImageView& image_view = image_views.emplace_back();
			VK_CHECK(vkCreateImageView(context_state->get_device(), &image_view_info, nullptr, &image_view));
		}

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
			sampler_create_info.minLod = 0.0f;
			sampler_create_info.maxLod = 16.0f;
			sampler_create_info.mipmapMode = config.linear_mip_filtering ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;

			VkSamplerReductionModeCreateInfo create_info_sampler_reduction = {};
			if (config.does_min_reduction)
			{
				create_info_sampler_reduction.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT;
				create_info_sampler_reduction.pNext = nullptr;
				create_info_sampler_reduction.reductionMode = VK_SAMPLER_REDUCTION_MODE_MAX;
				sampler_create_info.pNext = &create_info_sampler_reduction;
			}

			vkCreateSampler(context_state->get_device(), &sampler_create_info, nullptr, &sampler);
		}
	}

	void VulkanImage::initialize(class GraphicsContext* const gfx_context, const AttachmentConfig& config, void* image_handle, void* image_view_handle)
	{
		VkImage existing_image = static_cast<VkImage>(image_handle);
		VkImageView existing_image_view = static_cast<VkImageView>(image_view_handle);
		assert(existing_image != nullptr && existing_image_view != nullptr);

		VkImageView& image_view = image_views.emplace_back();

		image = existing_image;
		image_view = existing_image_view;
		access_flags = AccessFlags::None;
		layout = ImageLayout::Undefined;
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
		for (VkImageView& image_view : image_views)
		{
			vkDestroyImageView(context_state->get_device(), image_view, nullptr);
		}
		vmaDestroyImage(allocator, image, allocation);
	}

	void VulkanImage::copy_from_buffer(class GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, class Buffer* buffer, uint32_t buffer_offset, uint32_t mip_level, uint32_t array_layer, uint32_t array_count)
	{
		barrier(
			gfx_context,
			command_buffer,
			config,
			access_flags,
			AccessFlags::TransferWrite,
			layout,
			ImageLayout::TransferDestination,
			PipelineStageType::TopOfPipe,
			PipelineStageType::Transfer
		);

		{
			VkCommandBuffer cmd = static_cast<VkCommandBuffer>(command_buffer);

			VkBufferImageCopy buffer_image_copy = {};
			buffer_image_copy.bufferOffset = buffer_offset;
			buffer_image_copy.bufferRowLength = 0;
			buffer_image_copy.bufferImageHeight = 0;
			buffer_image_copy.imageSubresource.aspectMask = VK_FROM_SUNSET_IMAGE_USAGE_ASPECT_FLAGS(config.flags);
			buffer_image_copy.imageSubresource.mipLevel = mip_level;
			buffer_image_copy.imageSubresource.baseArrayLayer = array_layer;
			buffer_image_copy.imageSubresource.layerCount = array_count;
			buffer_image_copy.imageExtent = VkExtent3D{ static_cast<unsigned int>(config.extent.x) >> mip_level, static_cast<unsigned int>(config.extent.y) >> mip_level, static_cast<unsigned int>(glm::max(1.0f, config.extent.z)) };

			VkBuffer other_buffer = static_cast<VkBuffer>(buffer->get());
			vkCmdCopyBufferToImage(cmd, other_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);
		}

		barrier(
			gfx_context,
			command_buffer,
			config,
			AccessFlags::TransferWrite,
			AccessFlags::ShaderRead,
			ImageLayout::TransferDestination,
			ImageLayout::ShaderReadOnly,
			PipelineStageType::Transfer,
			PipelineStageType::FragmentShader
		);
	}

	void VulkanImage::copy_to_buffer(GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, Buffer* buffer, uint32_t buffer_offset, uint32_t mip_level, uint32_t array_layer, uint32_t array_count)
	{
		barrier(
			gfx_context,
			command_buffer,
			config,
			AccessFlags::None,
			AccessFlags::TransferRead,
			ImageLayout::Undefined,
			ImageLayout::TransferSource,
			PipelineStageType::TopOfPipe,
			PipelineStageType::Transfer
		);

		{
			VkCommandBuffer cmd = static_cast<VkCommandBuffer>(command_buffer);

			VkBufferImageCopy buffer_image_copy = {};
			buffer_image_copy.bufferOffset = buffer_offset;
			buffer_image_copy.bufferRowLength = 0;
			buffer_image_copy.bufferImageHeight = 0;
			buffer_image_copy.imageSubresource.aspectMask = VK_FROM_SUNSET_IMAGE_USAGE_ASPECT_FLAGS(config.flags);
			buffer_image_copy.imageSubresource.mipLevel = mip_level;
			buffer_image_copy.imageSubresource.baseArrayLayer = array_layer;
			buffer_image_copy.imageSubresource.layerCount = array_count;
			buffer_image_copy.imageExtent = VkExtent3D{ static_cast<unsigned int>(config.extent.x) >> mip_level, static_cast<unsigned int>(config.extent.y) >> mip_level, static_cast<unsigned int>(glm::max(1.0f, config.extent.z)) };

			VkBuffer other_buffer = static_cast<VkBuffer>(buffer->get());
			vkCmdCopyImageToBuffer(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, other_buffer, 1, &buffer_image_copy);
		}

		barrier(
			gfx_context,
			command_buffer,
			config,
			AccessFlags::TransferRead,
			AccessFlags::ShaderRead,
			ImageLayout::TransferSource,
			ImageLayout::ShaderReadOnly,
			PipelineStageType::Transfer,
			PipelineStageType::FragmentShader
		);
	}

	char* VulkanImage::map_gpu(GraphicsContext* const gfx_context)
	{
		assert(gfx_context->get_buffer_allocator() != nullptr);

		VmaAllocator allocator = static_cast<VmaAllocator>(gfx_context->get_buffer_allocator()->get_handle());

		char* mapped_memory;
		vmaMapMemory(allocator, allocation, (void**)&mapped_memory);

		return mapped_memory;
	}

	void VulkanImage::unmap_gpu(GraphicsContext* const gfx_context)
	{
		assert(gfx_context->get_buffer_allocator() != nullptr);

		VmaAllocator allocator = static_cast<VmaAllocator>(gfx_context->get_buffer_allocator()->get_handle());

		vmaUnmapMemory(allocator, allocation);
	}

	void VulkanImage::bind(class GraphicsContext* const gfx_context, void* command_buffer)
	{

	}

	void VulkanImage::barrier(class GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, AccessFlags src_access, AccessFlags dst_access, ImageLayout src_layout, ImageLayout dst_layout, PipelineStageType src_pipeline_stage, PipelineStageType dst_pipeline_stage)
	{
		VkImageMemoryBarrier image_barrier = {};
		image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_barrier.pNext = nullptr;
		image_barrier.srcAccessMask = VK_FROM_SUNSET_ACCESS_FLAGS(src_access);
		image_barrier.dstAccessMask = VK_FROM_SUNSET_ACCESS_FLAGS(dst_access);
		image_barrier.oldLayout = VK_FROM_SUNSET_IMAGE_LAYOUT_FLAGS(src_layout);
		image_barrier.newLayout = VK_FROM_SUNSET_IMAGE_LAYOUT_FLAGS(dst_layout);
		image_barrier.image = image;
		image_barrier.subresourceRange.levelCount = config.mip_count;
		image_barrier.subresourceRange.layerCount = config.array_count;
		image_barrier.subresourceRange.aspectMask = VK_FROM_SUNSET_IMAGE_USAGE_ASPECT_FLAGS(config.flags);
		image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		VkCommandBuffer cmd = static_cast<VkCommandBuffer>(command_buffer);
		vkCmdPipelineBarrier(
			cmd,
			VK_FROM_SUNSET_PIPELINE_STAGE_TYPE(src_pipeline_stage),
			VK_FROM_SUNSET_PIPELINE_STAGE_TYPE(dst_pipeline_stage),
			0,
			0, nullptr,
			0, nullptr,
			1, &image_barrier
		);

		access_flags = dst_access;
		layout = dst_layout;
	}

	void VulkanImage::blit(GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, Image* const other, const glm::vec3 src_blit_size, const glm::vec3 dst_blit_size, uint32_t src_mip, uint32_t dst_mip, int32_t src_layer, int32_t dst_layer)
	{
		assert(other != nullptr && "Cannot blit from a null image");
		
		const AttachmentConfig& other_config = other->get_attachment_config();

		VkImageBlit region;
		region.srcSubresource.aspectMask = VK_FROM_SUNSET_IMAGE_USAGE_ASPECT_FLAGS(config.flags);
		region.srcSubresource.mipLevel = src_mip;
		region.srcSubresource.baseArrayLayer = src_layer == -1 ? 0 : src_layer;
		region.srcSubresource.layerCount = src_layer == -1 ? config.array_count : 1;
		region.srcOffsets[0] = { .x = 0, .y = 0, .z = 0 };
		region.srcOffsets[1] = { .x = static_cast<int32_t>(src_blit_size.x), .y = static_cast<int32_t>(src_blit_size.y), .z = 1 };
		region.dstSubresource.aspectMask = VK_FROM_SUNSET_IMAGE_USAGE_ASPECT_FLAGS(other_config.flags);
		region.dstSubresource.mipLevel = dst_mip;
		region.dstSubresource.baseArrayLayer = dst_layer == -1 ? 0 : dst_layer;
		region.dstSubresource.layerCount = dst_layer == -1 ? other_config.array_count : 1;
		region.dstOffsets[0] = { .x = 0, .y = 0, .z = 0 };
		region.dstOffsets[1] = { .x = static_cast<int32_t>(dst_blit_size.x), .y = static_cast<int32_t>(dst_blit_size.y), .z = 1 };

		VkCommandBuffer cmd = static_cast<VkCommandBuffer>(command_buffer);
		vkCmdBlitImage(cmd, static_cast<VkImage>(other->get_image()), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_LINEAR);
	}

	void VulkanImage::clear(class GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, const glm::vec4& clear_color)
	{
		const bool b_is_depth_stencil = (config.flags & (ImageFlags::DepthStencil | ImageFlags::Depth | ImageFlags::Stencil)) != ImageFlags::None;

		VkImageSubresourceRange range = {};
		range.aspectMask = b_is_depth_stencil ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = config.mip_count;
		range.baseArrayLayer = 0;
		range.layerCount = config.array_count;

		if (b_is_depth_stencil)
		{
			VkClearDepthStencilValue vk_clear_color = { .depth = clear_color.r, .stencil = 0 };
			vkCmdClearDepthStencilImage(static_cast<VkCommandBuffer>(command_buffer), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vk_clear_color, 1, &range);
		}
		else
		{
			VkClearColorValue vk_clear_color = { .float32 = { clear_color.r, clear_color.g, clear_color.b, clear_color.a } };
			vkCmdClearColorImage(static_cast<VkCommandBuffer>(command_buffer), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vk_clear_color, 1, &range);
		}
	}
}
