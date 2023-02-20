#pragma once

#include <minimal.h>
#include <free_list_array.h>
#include <core/ecs/entity.h>

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

	struct EntitySceneData
	{
		glm::mat4 local_transform;
		glm::vec4 bounds_pos_radius;
		glm::vec4 bounds_extent;
		int32_t material_index{ -1 };
	};

	struct MaterialData
	{
		int32_t textures[MAX_MATERIAL_TEXTURES];
	};

	DECLARE_GPU_SHARED_DATA(EntitySceneData, MIN_ENTITIES);
	DECLARE_GPU_SHARED_DATA(MaterialData, MAX_MATERIALS);
}