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
}
