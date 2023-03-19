#include <graphics/api/vulkan/vk_barrier_batcher.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/image.h>

namespace Sunset
{
	void VulkanBarrierBatcher::begin(class GraphicsContext* const gfx_context, PipelineStageType stage)
	{
		current_stage = stage;
	}

	void VulkanBarrierBatcher::add_buffer_barrier(class GraphicsContext* const gfx_context, class Buffer* buffer, Identity execution_id, AccessFlags destination_access)
	{
		if (buffer_barriers.find(execution_id) == buffer_barriers.end())
		{
			buffer_barriers.insert({ execution_id, {} });
		}

		VkBufferMemoryBarrier buffer_barrier;
		buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		buffer_barrier.pNext = nullptr;
		buffer_barrier.srcAccessMask = VK_FROM_SUNSET_ACCESS_FLAGS(buffer->get_access_flags());
		buffer_barrier.dstAccessMask = VK_FROM_SUNSET_ACCESS_FLAGS(destination_access);
		buffer_barrier.buffer = static_cast<VkBuffer>(buffer->get());
		buffer_barrier.offset = 0;
		buffer_barrier.size = buffer->get_size();

		buffer_barriers[execution_id].push_back(buffer_barrier);
	}

	void VulkanBarrierBatcher::add_image_barrier(class GraphicsContext* const gfx_context, class Image* image, Identity execution_id, AccessFlags destination_access, ImageLayout final_layout)
	{
		if (image_barriers.find(execution_id) == image_barriers.end())
		{
			image_barriers.insert({ execution_id, {} });
		}

		VkImageSubresourceRange range;
		range.aspectMask = VK_FROM_SUNSET_IMAGE_USAGE_ASPECT_FLAGS(image->get_attachment_config().flags);
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		VkImageMemoryBarrier image_barrier;
		image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_barrier.pNext = nullptr;
		image_barrier.srcAccessMask = VK_FROM_SUNSET_ACCESS_FLAGS(image->get_access_flags());
		image_barrier.dstAccessMask = VK_FROM_SUNSET_ACCESS_FLAGS(destination_access);
		image_barrier.image = static_cast<VkImage>(image->get_image());
		image_barrier.oldLayout = VK_FROM_SUNSET_IMAGE_LAYOUT_FLAGS(image->get_layout());
		image_barrier.newLayout = VK_FROM_SUNSET_IMAGE_LAYOUT_FLAGS(final_layout);
		image_barrier.subresourceRange = range;

		image_barriers[execution_id].push_back(image_barrier);
	}

	void VulkanBarrierBatcher::execute(class GraphicsContext* const gfx_context, void* command_buffer, PipelineStageType stage, Identity execution_id)
	{
		if (execution_id.computed_hash != 0)
		{
			std::vector<VkBufferMemoryBarrier>& b_barriers = buffer_barriers[execution_id];
			std::vector<VkImageMemoryBarrier>& i_barriers = image_barriers[execution_id];

			if (!b_barriers.empty() || !i_barriers.empty())
			{
				vkCmdPipelineBarrier(
					static_cast<VkCommandBuffer>(command_buffer),
					VK_FROM_SUNSET_PIPELINE_STAGE_TYPE(current_stage),
					VK_FROM_SUNSET_PIPELINE_STAGE_TYPE(stage),
					0, 0, nullptr,
					static_cast<uint32_t>(b_barriers.size()),
					b_barriers.data(),
					static_cast<uint32_t>(i_barriers.size()),
					i_barriers.data()
				);

				current_stage = stage;
			}

			b_barriers.clear();
			i_barriers.clear();
		}
		else
		{
			std::vector<VkBufferMemoryBarrier> b_barriers;
			std::vector<VkImageMemoryBarrier> i_barriers;

			for (auto& [id, barriers] : buffer_barriers)
			{
				b_barriers.insert(b_barriers.end(), barriers.begin(), barriers.end());
				barriers.clear();
			}
			for (auto& [id, barriers] : image_barriers)
			{
				i_barriers.insert(i_barriers.end(), barriers.begin(), barriers.end());
				barriers.clear();
			}

			if (!b_barriers.empty() || !i_barriers.empty())
			{
				vkCmdPipelineBarrier(
					static_cast<VkCommandBuffer>(command_buffer),
					VK_FROM_SUNSET_PIPELINE_STAGE_TYPE(current_stage),
					VK_FROM_SUNSET_PIPELINE_STAGE_TYPE(stage),
					0, 0, nullptr,
					static_cast<uint32_t>(b_barriers.size()),
					b_barriers.data(),
					static_cast<uint32_t>(i_barriers.size()),
					i_barriers.data()
				);

				current_stage = stage;
			}
		}
	}

	void VulkanBarrierBatcher::reset()
	{
		current_stage = PipelineStageType::TopOfPipe;
	}
}
