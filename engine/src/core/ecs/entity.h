#pragma once

#include <minimal.h>
#include <core/ecs/component.h>
#include <gpu_shared_data_types.h>

namespace Sunset
{
	using EntityIndex = uint32_t;
	using EntityVersion = uint32_t;
	using EntityID = uint64_t;

	struct Entity
	{
		EntityID id;
		ComponentMask components;
	};

	struct alignas(16) EntitySceneData
	{
		glm::mat4 local_transform;
		glm::vec4 bounds_pos_radius;
		glm::vec4 bounds_extent;
		int32_t material_index{ -1 };
		int32_t padding[3];
	};

	DECLARE_GPU_SHARED_DATA(EntitySceneData, MIN_ENTITIES);

	inline EntityID create_entity_id(EntityIndex index, EntityVersion version)
	{
		return ((EntityID)index << 32) | ((EntityID)version);
	}

	inline EntityIndex get_entity_index(EntityID id)
	{
		return id >> 32;
	}

	inline EntityVersion get_entity_version(EntityID id)
	{
		return (EntityVersion)id;
	}

	inline bool is_valid_entity(EntityID id)
	{
		return (id >> 32) != EntityIndex(-1);
	}

	#define INVALID_ENTITY create_entity_id(EntityIndex(-1), 0)
}
