#pragma once

#include <minimal.h>
#include <core/ecs/component.h>

namespace Sunset
{
	const int MIN_ENTITIES = 2000;

	using EntityIndex = uint32_t;
	using EntityVersion = uint32_t;
	using EntityID = uint64_t;

	struct Entity
	{
		EntityID id;
		ComponentMask components;
	};

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
