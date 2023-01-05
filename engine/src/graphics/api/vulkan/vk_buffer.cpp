#include <graphics/api/vulkan/vk_buffer.h>
#include <graphics/resource/buffer.h>
#include <graphics/graphics_context.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace Sunset
{
	inline VkBufferUsageFlagBits SUNSET_TO_VULKAN_BUFFER_TYPE(BufferType type)
	{
		int32_t usage_flags{ 0 };
		if (static_cast<int32_t>(type & BufferType::Vertex) > 0)
		{
			usage_flags = usage_flags | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		}
		if (static_cast<int32_t>(type & BufferType::UniformBuffer) > 0)
		{
			usage_flags = usage_flags | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		}
		if (static_cast<int32_t>(type & BufferType::StorageBuffer) > 0)
		{
			usage_flags = usage_flags | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		}
		if (static_cast<int32_t>(type & BufferType::TransferSource) > 0)
		{
			usage_flags = usage_flags | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}
		if (static_cast<int32_t>(type & BufferType::TransferDestination) > 0)
		{
			usage_flags = usage_flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
		return usage_flags > 0 ? static_cast<VkBufferUsageFlagBits>(usage_flags) : VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}

	void VulkanBufferAllocator::initialize(class GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		VmaAllocatorCreateInfo allocator_info = {};
		allocator_info.physicalDevice = context_state->get_gpu();
		allocator_info.device = context_state->get_device();
		allocator_info.instance = context_state->instance;

		vmaCreateAllocator(&allocator_info, &allocator);
	}


	void VulkanBufferAllocator::destroy()
	{
		vmaDestroyAllocator(allocator);
	}

	void VulkanBuffer::initialize(class GraphicsContext* const gfx_context, size_t buffer_size, BufferType type, MemoryUsageType memory_usage)
	{
		assert(gfx_context->get_buffer_allocator() != nullptr);

		VmaAllocator allocator = static_cast<VmaAllocator>(gfx_context->get_buffer_allocator()->get_handle());

		VkBufferCreateInfo buffer_create_info = {};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = buffer_size;
		buffer_create_info.usage = SUNSET_TO_VULKAN_BUFFER_TYPE(type);

		VmaAllocationCreateInfo allocation_create_info = {};
		allocation_create_info.usage = VK_FROM_SUNSET_MEMORY_USAGE(memory_usage);

		VK_CHECK(vmaCreateBuffer(allocator, &buffer_create_info, &allocation_create_info, &buffer, &allocation, nullptr));

		size = buffer_size;
	}

	void VulkanBuffer::destroy(class GraphicsContext* const gfx_context)
	{
		assert(gfx_context->get_buffer_allocator() != nullptr);

		VmaAllocator allocator = static_cast<VmaAllocator>(gfx_context->get_buffer_allocator()->get_handle());

		vmaDestroyBuffer(allocator, buffer, allocation);

		buffer = nullptr;
	}

	void VulkanBuffer::copy_from(GraphicsContext* const gfx_context, void* data, size_t buffer_size, size_t buffer_offset)
	{
		assert(gfx_context->get_buffer_allocator() != nullptr);

		VmaAllocator allocator = static_cast<VmaAllocator>(gfx_context->get_buffer_allocator()->get_handle());

		char* mapped_memory;
		vmaMapMemory(allocator, allocation, (void**)&mapped_memory);
		mapped_memory += buffer_offset;
		memcpy(mapped_memory, data, buffer_size);
		vmaUnmapMemory(allocator, allocation);
	}

	void VulkanBuffer::copy_buffer(class GraphicsContext* const gfx_context, void* command_buffer, Buffer* other, size_t buffer_size, size_t buffer_offset /*= 0*/)
	{
		VkBuffer other_buffer = static_cast<VkBuffer>(other->get());
		VkCommandBuffer cmd = static_cast<VkCommandBuffer>(command_buffer);

		VkBufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = buffer_size;

		vkCmdCopyBuffer(cmd, other_buffer, buffer, 1, &copy);
	}

	void VulkanBuffer::bind(GraphicsContext* const gfx_context, BufferType type, void* command_buffer)
	{
		assert(gfx_context->get_buffer_allocator() != nullptr);

		if (static_cast<int32_t>(type & BufferType::Vertex) > 0)
		{
			VkDeviceSize offset{ 0 };
			vkCmdBindVertexBuffers(static_cast<VkCommandBuffer>(command_buffer), 0, 1, &buffer, &offset);
		}
	}
}
