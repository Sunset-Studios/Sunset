#pragma once

#include <minimal.h>
#include <free_list_array.h>

// In this context, "shared" structs are structs that can exist in
// GPU visible buffers at any given time
namespace Sunset
{
	template<class DataType, int SharedDataCount = 512>
	struct GPUSharedData
	{
		BufferID data_buffer[MAX_BUFFERED_FRAMES];
		FreeListArray<DataType> data{ SharedDataCount };

		DataType& operator[](uint32_t index)
		{
			return data[index];
		}
	};

	#define DECLARE_GPU_SHARED_DATA(Type, Count) using Type##Shared = GPUSharedData<Type, Count>
}