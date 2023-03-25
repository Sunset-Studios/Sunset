#include <graphics/resource/material.h>
#include <graphics/asset_pool.h>
#include <graphics/graphics_context.h>
#include <graphics/pipeline_state.h>
#include <graphics/descriptor.h>
#include <core/data_globals.h>
#include <graphics/resource/image.h>
#include <graphics/renderer.h>
#include <graphics/render_pass.h>

namespace Sunset
{
	void material_load_textures(class GraphicsContext* const gfx_context, MaterialID material)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot load material textures for a null material!");

		material_ptr->textures.resize(material_ptr->description.textures.size());
		for (int i = 0; i < material_ptr->textures.size(); ++i)
		{
			if (material_ptr->description.textures[i] != nullptr)
			{
				material_ptr->textures[i] = ImageFactory::load(
					gfx_context,
					{
						.name = material_ptr->description.textures[i],
						.path = material_ptr->description.textures[i],
						.flags = (ImageFlags::Sampled | ImageFlags::TransferDst),
						.usage_type = MemoryUsageType::OnlyGPU,
						.image_filter = ImageFilter::Nearest
					}
				);
				material_ptr->b_needs_texture_upload = true;
			}
		}
	}

	void material_upload_textures(class GraphicsContext* const gfx_context, MaterialID material, class DescriptorSet* descriptor_set)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot load material textures for a null material!");
		
		if (material_ptr->b_needs_texture_upload)
		{
			const uint32_t current_buffered_frame = gfx_context->get_buffered_frame_number();

			std::vector<DescriptorBindlessWrite> bindless_writes;
			for (int i = 0; i < material_ptr->textures.size(); ++i)
			{
				if (const ImageID texture = material_ptr->textures[i]; texture != 0)
				{
					bindless_writes.push_back(
						DescriptorBindlessWrite{
							.slot = ImageBindTableSlot,
							.type = DescriptorType::Image,
							.buffer = CACHE_FETCH(Image, texture),
							.set = descriptor_set
						}
					);
				}
			}

			DescriptorHelpers::write_bindless_descriptors(gfx_context, bindless_writes, material_ptr->gpu_data->textures);

			// Update mapped SSBO data
			Buffer* const material_buffer = CACHE_FETCH(Buffer, MaterialGlobals::get()->material_data.data_buffer[current_buffered_frame]);
			material_buffer->copy_from(
				gfx_context,
				material_ptr->gpu_data,
				sizeof(MaterialData),
				sizeof(MaterialData) * material_ptr->gpu_data_buffer_offset
			);

			material_ptr->b_needs_texture_upload = false;
		}
	}

	void material_track_gpu_shared_data(class GraphicsContext* const gfx_context, MaterialID material)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot track null material!");

		if (material_ptr->gpu_data == nullptr)
		{
			material_ptr->gpu_data = MaterialGlobals::get()->new_shared_data();
			material_ptr->gpu_data_buffer_offset = MaterialGlobals::get()->get_shared_data_index(material_ptr->gpu_data);
			for (uint32_t i = 0; i < MAX_MATERIAL_TEXTURES; ++i)
			{
				material_ptr->gpu_data->textures[i] = -1;
			}
		}
	}

	Sunset::MaterialID MaterialFactory::create(class GraphicsContext* const gfx_context, const MaterialDescription& desc)
	{
		Identity cache_id{ static_cast<uint32_t>(std::hash<MaterialDescription>{}(desc)) };
		bool b_added{ false };
		MaterialID material_id = MaterialCache::get()->fetch_or_add(cache_id, gfx_context, b_added);
		if (b_added)
		{
			CACHE_FETCH(Material, material_id)->description = desc;
			material_load_textures(gfx_context, material_id);
			material_track_gpu_shared_data(gfx_context, material_id);
		}
		return material_id;
	}
}
