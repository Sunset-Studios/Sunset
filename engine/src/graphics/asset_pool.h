#pragma once

#include <minimal.h>
#include <memory/allocators/pool_allocator.h>

namespace Sunset
{
	// Size default
	template<class T, size_t Size = 512>
	struct PoolSize
	{
		static constexpr int value = Size;
	};

	// Size specializations
	template<>
	struct PoolSize<struct Mesh>
	{
		static constexpr int value = 512;
	};

	template<>
	struct PoolSize<class PipelineState>
	{
		static constexpr int value = 512;
	};

	template<class T>
	class GlobalAssetPools
	{
		public:
			static StaticPoolAllocator<T, PoolSize<T>::value>* get()
			{
				static std::unique_ptr<StaticPoolAllocator<T, PoolSize<T>::value>> pool = std::make_unique<StaticPoolAllocator<T, PoolSize<T>::value>>();
				return pool.get();
			}
	};
}
