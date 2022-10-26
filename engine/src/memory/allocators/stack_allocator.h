#pragma once

#include <list>
#include <memory_resource>
#include <cassert>

namespace Sunset
{
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

		T* get_new()
		{
			assert(current_frame_index < max_items_in_pool && "There are no more elements in the pool to allocate from! Try increasing the frame allocator size.");
			T* obj = &items[current_frame_index++];
			return obj;
		}

	protected:
		std::pmr::monotonic_buffer_resource buffer_resource{ max_items_in_pool_byte };
		std::pmr::polymorphic_allocator<T> allocator{ &buffer_resource };
		std::pmr::vector<T> items{ allocator };
		std::atomic_int32_t current_frame_index{ 0 };
	};
}
