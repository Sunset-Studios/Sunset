#pragma once

#include <common.h>

namespace Sunset
{
	enum class BufferType : int16_t
	{
		Generic,
		Vertex
	};

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

		void initialize(class GraphicsContext* const gfx_context, size_t buffer_size, BufferType type)
		{
			buffer_policy.initialize(gfx_context, buffer_size, type);
		}

		void copy_from(class GraphicsContext* const gfx_context, void* data, size_t buffer_size)
		{
			buffer_policy.copy_from(gfx_context, data, buffer_size);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			buffer_policy.destroy(gfx_context);
		}

	private:
		Policy buffer_policy;
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

		void initialize(class GraphicsContext* const gfx_context, size_t buffer_size)
		{ }

		void destroy(class GraphicsContext* const gfx_context)
		{ }

		void copy_from(class GraphicsContext* const gfx_context, void* data, size_t buffer_size)
		{ }
	};

#if USE_VULKAN_GRAPHICS
	class Buffer : public GenericBuffer<VulkanBuffer>
	{ };
	class BufferAllocator : public GenericBufferAllocator<VulkanBuffer>
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
		static Buffer* create(class GraphicsContext* const gfx_context, size_t buffer_size, BufferType type)
		{
			Buffer* buffer = new Buffer;
			buffer->initialize(gfx_context, buffer_size, type);
			return buffer;
		}
	};
}
