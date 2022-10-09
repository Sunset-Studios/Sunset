#include <graphics/api/vulkan/vk_buffer.h>
#include <graphics/resource/buffer.h>
#include <graphics/graphics_context.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace Sunset
{
	inline VkBufferUsageFlagBits SUNSET_TO_VULKAN_BUFFER_TYPE(BufferType type)
	{
		switch (type)
		{
			case BufferType::Vertex:
				return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				break;
			case BufferType::Generic:
			default:
				return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		}
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

	void VulkanBuffer::initialize(class GraphicsContext* const gfx_context, size_t buffer_size, BufferType type)
	{
		assert(gfx_context->get_buffer_allocator() != nullptr);

		VmaAllocator allocator = static_cast<VmaAllocator>(gfx_context->get_buffer_allocator()->get_handle());

		VkBufferCreateInfo buffer_create_info = {};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = buffer_size;
		buffer_create_info.usage = SUNSET_TO_VULKAN_BUFFER_TYPE(type);

		VmaAllocationCreateInfo allocation_create_info = {};
		allocation_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VK_CHECK(vmaCreateBuffer(allocator, &buffer_create_info, &allocation_create_info, &buffer, &allocation, nullptr));
	}

	void VulkanBuffer::destroy(class GraphicsContext* const gfx_context)
	{
		assert(gfx_context->get_buffer_allocator() != nullptr);

		VmaAllocator allocator = static_cast<VmaAllocator>(gfx_context->get_buffer_allocator()->get_handle());

		vmaDestroyBuffer(allocator, buffer, allocation);
	}

	void VulkanBuffer::copy_from(GraphicsContext* const gfx_context, void* data, size_t buffer_size)
	{
		assert(gfx_context->get_buffer_allocator() != nullptr);

		VmaAllocator allocator = static_cast<VmaAllocator>(gfx_context->get_buffer_allocator()->get_handle());

		void* mapped_memory;
		vmaMapMemory(allocator, allocation, &mapped_memory);
		memcpy(mapped_memory, data, buffer_size);
		vmaUnmapMemory(allocator, allocation);
	}

	void VulkanBuffer::bind(GraphicsContext* const gfx_context, BufferType type, void* command_buffer)
	{
		assert(gfx_context->get_buffer_allocator() != nullptr);

		switch (type)
		{
			case BufferType::Vertex:
			{
				VkDeviceSize offset{ 0 };
				vkCmdBindVertexBuffers(static_cast<VkCommandBuffer>(command_buffer), 0, 1, &buffer, &offset);
				break;
			}
			case BufferType::Generic:
			default:
				break;
		}
	}
}
