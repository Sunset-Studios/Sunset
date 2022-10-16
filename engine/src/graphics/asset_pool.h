#pragma once

#include <minimal.h>
#include <memory/allocators/pool_allocator.h>

namespace Sunset
{
	template<class T>
	struct PoolSize
	{
		static int value;
	};

	template<class T>
	int PoolSize<T>::value = 512;

	template<> int PoolSize<class Mesh>::value = 512;

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
