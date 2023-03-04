#pragma once

#include <minimal.h>
#include <singleton.h>
#include <bit_vector.h>
#include <entity.h>
#include <gpu_shared_data_types.h>

namespace Sunset
{
	class EntityGlobals : public Singleton<EntityGlobals>
	{
	friend class Singleton;

	public:
		void initialize() { }

	public:
		EntitySceneDataShared entity_data;
		BitVector entity_transform_dirty_states{ MIN_ENTITIES };
	};

	class MaterialGlobals : public Singleton<MaterialGlobals>
	{
		friend class Singleton;

	public:
		void initialize() { }

		MaterialData* new_shared_data();
		uint32_t get_shared_data_index(MaterialData* data);

	public:
		MaterialDataShared material_data;
	};
}
