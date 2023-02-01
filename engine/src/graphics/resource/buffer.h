#pragma once

#include <common.h>
#include <buffer_types.h>
#include <graphics/resource/resource_cache.h>

namespace Sunset
{
	template<class Policy>
	class GenericBufferAllocator
	{
	public:
		GenericBufferAllocator() = default;

		void initialize(class GraphicsContext* const gfx_context)
		{
			allocator_policy.initialize(gfx_context);
		}

		void destroy()
		{
			allocator_policy.destroy();
		}

		void* get_handle()
		{
			return allocator_policy.get_handle();
		}

	private:
		Policy allocator_policy;
	};

	template<class Policy>
	class GenericBuffer
	{
	public:
		GenericBuffer() = default;

		void initialize(class GraphicsContext* const gfx_context, const BufferConfig& config)
		{
			buffer_config = config;
			buffer_policy.initialize(gfx_context, buffer_config);
		}

		void reallocate(class GraphicsContext* const gfx_context, size_t new_buffer_size)
		{
			buffer_config.buffer_size = new_buffer_size;
			buffer_policy.reallocate(gfx_context, buffer_config);
		}

		void copy_from(class GraphicsContext* const gfx_context, void* data, size_t buffer_size, size_t buffer_offset = 0, std::function<void(void*)> memcpy_op = {})
		{
			buffer_policy.copy_from(gfx_context, data, buffer_size, buffer_offset, memcpy_op);
		}

		void copy_buffer(class GraphicsContext* const gfx_context, void* command_buffer, class Buffer* other, size_t buffer_size, size_t buffer_offset = 0)
		{
			buffer_policy.copy_buffer(gfx_context, command_buffer, other, buffer_size, buffer_offset);
		}

		char* map_gpu(class GraphicsContext* const gfx_context)
		{
			return buffer_policy.map_gpu(gfx_context);
		}

		void unmap_gpu(class GraphicsContext* const gfx_context)
		{
			buffer_policy.unmap_gpu(gfx_context);
		}

		void bind(class GraphicsContext* const gfx_context, void* command_buffer)
		{
			buffer_policy.bind(gfx_context, buffer_type, command_buffer);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			buffer_policy.destroy(gfx_context);
		}

		void barrier(class GraphicsContext* const gfx_context, void* command_buffer, AccessFlags src_access, AccessFlags dst_access, PipelineStageType src_pipeline_stage, PipelineStageType dst_pipeline_stage)
		{
			buffer_policy.barrier(gfx_context, command_buffer, src_access, dst_access, src_pipeline_stage, dst_pipeline_stage);
		}

		void* get()
		{
			return buffer_policy.get();
		}

		size_t get_size() const
		{
			return buffer_policy.get_size();
		}

	private:
		Policy buffer_policy;
		BufferConfig buffer_config;
	};

	class NoopBufferAllocator
	{
	public:
		NoopBufferAllocator() = default;

		void initialize(class GraphicsContext* const gfx_context)
		{ }

		void destroy()
		{ }

		void* get_handle()
		{
			return nullptr;
		}
	};

	class NoopBuffer
	{
	public:
		NoopBuffer() = default;

		void initialize(class GraphicsContext* const gfx_context, const BufferConfig& config)
		{ }

		void reallocate(class GraphicsContext* const gfx_context, const BufferConfig& config)
		{ }

		void destroy(class GraphicsContext* const gfx_context)
		{ }

		void copy_from(class GraphicsContext* const gfx_context, void* data, size_t buffer_size, size_t buffer_offset = 0, std::function<void(void*)> memcpy_op = {})
		{ }

		void copy_buffer(class GraphicsContext* const gfx_context, void* command_buffer, class Buffer* other, size_t buffer_size, size_t buffer_offset = 0)
		{ }

		char* map_gpu(class GraphicsContext* const gfx_context)
		{
			return nullptr;
		}

		void unmap_gpu(class GraphicsContext* const gfx_context)
		{ }

		void bind(class GraphicsContext* const gfx_context, BufferType type, void* command_buffer, AccessFlags src_access, AccessFlags dst_access, PipelineStageType src_pipeline_stage, PipelineStageType dst_pipeline_stage)
		{ }

		void barrier(class GraphicsContext* const gfx_context, void* command_buffer)
		{ }

		void* get()
		{
			return nullptr;
		}

		size_t get_size() const
		{
			return 0;
		}
	};

#if USE_VULKAN_GRAPHICS
	class Buffer : public GenericBuffer<VulkanBuffer>
	{ };
	class BufferAllocator : public GenericBufferAllocator<VulkanBufferAllocator>
	{ };
#else
	class Buffer : public GenericBuffer<NoopBuffer>
	{ };
	class BufferAllocator : public GenericBufferAllocator<NoopBufferAllocator>
	{ };
#endif

	class BufferAllocatorFactory
	{
	public:
		static BufferAllocator* create(class GraphicsContext* const gfx_context)
		{
			BufferAllocator* buffer_alloc = new BufferAllocator;
			buffer_alloc->initialize(gfx_context);
			return buffer_alloc;
		}
	};

	class BufferFactory
	{
	public:
		static BufferID create(class GraphicsContext* const gfx_context, const BufferConfig& config, bool auto_delete = true);
	};

	DEFINE_RESOURCE_CACHE(BufferCache, BufferID, Buffer);

	class BufferHelpers
	{
	public:
		static size_t pad_ubo_size(size_t ubo_size, size_t min_ubo_alignment);
	};

	struct ScopedGPUBufferMapping
	{
		ScopedGPUBufferMapping(class GraphicsContext* const gfx_context, Buffer* buffer);
		~ScopedGPUBufferMapping();

		class GraphicsContext* gfx_context{ nullptr };
		char* mapped_memory{ nullptr };
		Buffer* buffer{ nullptr };
	};
}
