#pragma once

#include <vk_types.h>
#include <vk_initializers.h>
#include <vk_mem_alloc.h>

namespace Sunset
{
	class VulkanBufferAllocator
	{
	public:
		VulkanBufferAllocator() = default;

		void initialize(class GraphicsContext* const gfx_context);
		void destroy();

		void* get_handle()
		{
			return allocator;
		}

	protected:
		VmaAllocator allocator;
	};

	class VulkanBuffer
	{
	public:
		VulkanBuffer() = default;

		void initialize(class GraphicsContext* const gfx_context, size_t buffer_size, BufferType type);
		void destroy(class GraphicsContext* const gfx_context);
		void copy_from(class GraphicsContext* const gfx_context, void* data, size_t buffer_size);
		void bind(class GraphicsContext* const gfx_context, BufferType type, void* command_buffer);

	protected:
		VkBuffer buffer;
		VmaAllocation allocation;
	};
}
