#pragma once

#include <list>
#include <memory_resource>
#include <cassert>

namespace Sunset
{
	class monotonic_ring_buffer_resource : public std::pmr::memory_resource
	{
	public:
		monotonic_ring_buffer_resource() = default;

		explicit monotonic_ring_buffer_resource(memory_resource* const upstream) noexcept
			: resource(upstream)
		{ }

		explicit monotonic_ring_buffer_resource(const size_t size) noexcept
			: buffer_size(size)
		{ }

		monotonic_ring_buffer_resource(const size_t size, memory_resource* const upstream) noexcept
			: buffer_size(size), resource(upstream)
		{ }

		monotonic_ring_buffer_resource(void* const buffer, const size_t size, memory_resource* const upstream) noexcept
			: buffer(buffer), buffer_size(size), resource(upstream)
		{ }

		monotonic_ring_buffer_resource(void* const buffer, const size_t size) noexcept
			: buffer(buffer), buffer_size(size)
		{ }

		~monotonic_ring_buffer_resource() noexcept override
		{
			assert(buffer != nullptr && "Trying to destroy a monotonic_frame_buffer_resource with a null underlying buffer!");
			resource->deallocate(buffer, buffer_size, base_alignment);
			buffer = nullptr;
			current_buffer_offset = 0;
			buffer_size = 0;
		}

		monotonic_ring_buffer_resource(const monotonic_ring_buffer_resource&) = delete;
		monotonic_ring_buffer_resource& operator=(const monotonic_ring_buffer_resource&) = delete;

		void reset() noexcept
		{
			current_buffer_offset = 0;
		}

	private:
		virtual void* do_allocate(size_t bytes, size_t align)
		{
			if (buffer == nullptr)
			{
				buffer = resource->allocate(buffer_size, align);
				base_alignment = align;
			}

			void* const result = static_cast<char*>(buffer) + current_buffer_offset;
			current_buffer_offset = current_buffer_offset + bytes;
			current_buffer_offset = current_buffer_offset >= buffer_size ? 0 : current_buffer_offset;
			return result;
		}

		virtual void do_deallocate(void* ptr, size_t bytes, size_t align)
		{
			// Do nothing. The resource is monotonic and cyclical, so memory will be reused again once we call reset;
		}

		virtual bool do_is_equal(const memory_resource& that) const noexcept
		{
			return this == &that;
		};

	private:
		void* buffer{ nullptr };
		size_t buffer_size{ 0 };
		size_t base_alignment{ 0 };
		size_t current_buffer_offset{ 0 };
		memory_resource* resource{ std::pmr::get_default_resource() };
	};

	template<typename T, uint32_t PoolSize = 512>
	class SlimStaticRingAllocator
	{
		static const size_t max_items_in_pool = PoolSize;
		static const size_t max_items_in_pool_byte = PoolSize * sizeof(T);

	public:
		SlimStaticRingAllocator() = default;
		~SlimStaticRingAllocator() = default;

		std::pmr::polymorphic_allocator<T> get() const noexcept
		{
			return allocator;
		}

		void reset()
		{
			buffer_resource.reset();
		}

	protected:
		monotonic_ring_buffer_resource buffer_resource{ max_items_in_pool_byte };
		std::pmr::polymorphic_allocator<T> allocator{ &buffer_resource };
	};

	template<typename T, uint32_t PoolSize = 512>
	class StaticFrameAllocator
	{
		static const size_t max_items_in_pool = PoolSize;
		static const size_t max_items_in_pool_byte = PoolSize * sizeof(T);

	public:
		StaticFrameAllocator()
		{ 
			items.resize(max_items_in_pool);
		}
		~StaticFrameAllocator() = default;

		void reset()
		{
			current_frame_index = 0;
		}

		size_t size() const
		{
			return current_frame_index;
		}

		T* get_new()
		{
			assert(current_frame_index < max_items_in_pool && "There are no more elements in the pool to allocate from! Try increasing the frame allocator size.");
			T* obj = &items[current_frame_index++];
			return obj;
		}

		T* data()
		{
			return items.data();
		}

	protected:
		std::pmr::monotonic_buffer_resource buffer_resource{ max_items_in_pool_byte };
		std::pmr::polymorphic_allocator<T> allocator{ &buffer_resource };
		std::pmr::vector<T> items{ allocator };
		std::atomic_int32_t current_frame_index{ 0 };
	};
}
