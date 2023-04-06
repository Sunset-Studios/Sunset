#include "core/data_globals.h"

namespace Sunset
{
	Sunset::MaterialData* MaterialGlobals::new_shared_data()
	{
		return material_data.data.has_free() ? material_data.data.get_new() : nullptr;
	}

	uint32_t MaterialGlobals::get_shared_data_index(MaterialData* data)
	{
		return material_data.data.index_of(data);
	}

	void MaterialGlobals::release_shared_data(MaterialData* data)
	{
		material_data.data.free(data);
	}

	Sunset::LightData* LightGlobals::new_shared_data()
	{
		return light_data.data.has_free() ? light_data.data.get_new() : nullptr;
	}

	uint32_t LightGlobals::get_shared_data_index(LightData* data)
	{
		return light_data.data.index_of(data);
	}

	void LightGlobals::release_shared_data(LightData* data)
	{
		light_data.data.free(data);
	}
}
