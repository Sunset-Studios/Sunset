#pragma once

#include <list>
#include <memory_resource>
#include <cassert>

namespace Sunset
{
	template<typename T, uint32_t PoolSize = 512>
	class StaticPoolAllocator
	{
		static const size_t max_items_in_pool = PoolSize;
		static const size_t max_items_in_pool_byte = PoolSize * sizeof(T);

		public:
			StaticPoolAllocator()
				: total_free(max_items_in_pool)
			{ }
			~StaticPoolAllocator() = default;

			T* allocate()
			{
				assert(total_free > 0 && "There are no more elements in the pool to allocate from! Try increasing the pool size.");
				--total_free;
				return allocator.allocate(1);
			}

			void deallocate(T*& element)
			{
				assert(total_free < max_items_in_pool && "This deallocate call is returning an element to the pool that will cause a memory overflow! Make sure your alloc/dealloc calls are balanced while using this allocator.");
				++total_free;
				allocator.deallocate(element, 1);
				element = nullptr;
			}

		protected:
			std::pmr::monotonic_buffer_resource buffer_resource{ max_items_in_pool_byte };
			std::pmr::polymorphic_allocator<T> allocator{ &buffer_resource };
			uint32_t total_free{ 0 };
	};

	class StaticBytePoolAllocator
	{
	public:
		StaticBytePoolAllocator(size_t item_size, size_t max_item_count)
			: item_size(item_size), max_item_count(max_item_count)
		{
			std::pmr::monotonic_buffer_resource buffer_resource{ max_item_count * item_size };
			std::pmr::polymorphic_allocator<std::byte> allocator{ &buffer_resource };
			pool = std::pmr::vector<std::byte>{ max_item_count * item_size, std::byte(), allocator};
		}
		~StaticBytePoolAllocator() = default;

		inline void* get(size_t index)
		{
			assert(index >= 0 && index < max_item_count && "Trying to get an out-of-range pool alloc'd item! Check you index!");
			return pool.data() + index * item_size;
		}

	protected:
		size_t item_size;
		size_t max_item_count;
		std::pmr::vector<std::byte> pool;
	};
}
