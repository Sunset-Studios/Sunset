#pragma once

#include <minimal.h>
#include <singleton.h>
#include <bit_vector.h>
#include <entity.h>
#include <material.h>
#include <light.h>

namespace Sunset
{
	class EntityGlobals : public Singleton<EntityGlobals>
	{
	friend class Singleton;

	public:
		void initialize() { }

	public:
		EntitySceneDataShared entity_data;
		BitVector<MIN_ENTITIES> entity_transform_dirty_states;
	};

	class MaterialGlobals : public Singleton<MaterialGlobals>
	{
		friend class Singleton;

	public:
		void initialize() { }

		MaterialData* new_shared_data();
		uint32_t get_shared_data_index(MaterialData* data);
		void release_shared_data(MaterialData* data);

	public:
		MaterialDataShared material_data;
	};

	class LightGlobals : public Singleton<LightGlobals>
	{
		friend class Singleton;

	public:
		void initialize() { }

		LightData* new_shared_data();
		uint32_t get_shared_data_index(LightData* data);
		void release_shared_data(LightData* data);

	public:
		LightDataShared light_data;
		BitVector<MAX_LIGHT_COUNT> light_dirty_states;
	};
}
