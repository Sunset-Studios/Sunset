#pragma once

#include <vk_types.h>
#include <vk_initializers.h>
#include <vk_mem_alloc.h>

#include <graphics/resource/buffer_types.h>

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

		void initialize(class GraphicsContext* const gfx_context, const BufferConfig& config);
		void reallocate(class GraphicsContext* const gfx_context, const BufferConfig& config);
		void destroy(class GraphicsContext* const gfx_context);
		void copy_from(class GraphicsContext* const gfx_context, void* data, size_t buffer_size, size_t buffer_offset = 0, std::function<void(void*)> memcpy_op = {});
		void copy_buffer(class GraphicsContext* const gfx_context, void* command_buffer, class Buffer* other, size_t buffer_size, size_t buffer_offset = 0);
		char* map_gpu(class GraphicsContext* const gfx_context);
		void unmap_gpu(class GraphicsContext* const gfx_context);
		void bind(class GraphicsContext* const gfx_context, BufferType type, void* command_buffer);
		void barrier(class GraphicsContext* const gfx_context, void* command_buffer, AccessFlags src_access, AccessFlags dst_access, PipelineStageType src_pipeline_stage, PipelineStageType dst_pipeline_stage);
		void* get()
		{
			return buffer;
		}
		size_t get_size() const
		{
			return size;
		}

	protected:
		size_t size;
		VkBuffer buffer;
		VmaAllocation allocation;
	};
}
