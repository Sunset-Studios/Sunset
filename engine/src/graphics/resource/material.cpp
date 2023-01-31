#include <graphics/resource/material.h>
#include <graphics/asset_pool.h>
#include <graphics/graphics_context.h>
#include <graphics/pipeline_state.h>
#include <graphics/descriptor.h>

namespace Sunset
{
	void bind_material_pipeline(class GraphicsContext* const gfx_context, void* cmd_buffer, MaterialID material)
	{
		Material* const material_ptr = MaterialCache::get()->fetch(material);
		assert(material_ptr != nullptr && "Cannot bind pipeline for null material!");
		PipelineStateCache::get()->fetch(material_ptr->pipeline_state)->bind(gfx_context, cmd_buffer);
	}

	Sunset::PipelineStateID get_material_pipeline(MaterialID material)
	{
		Material* const material_ptr = MaterialCache::get()->fetch(material);
		assert(material_ptr != nullptr && "Cannot get pipeline for null material!");
		return material_ptr->pipeline_state;
	}

	void bind_material_descriptors(class GraphicsContext* const gfx_context, void* cmd_buffer, MaterialID material)
	{
		Material* const material_ptr = MaterialCache::get()->fetch(material);
		assert(material_ptr != nullptr && "Cannot bind descriptors for null material!");

		for (int i = 0; i < MAX_BOUND_DESCRIPTOR_SETS; ++i)
		{
			if (material_ptr->descriptor_datas[i].descriptor_set != nullptr)
			{
				material_ptr->descriptor_datas[i].descriptor_set->bind(gfx_context, cmd_buffer, material_ptr->pipeline_state, material_ptr->descriptor_datas[i].dynamic_buffer_offsets, i);
			}
		}
	}
}
