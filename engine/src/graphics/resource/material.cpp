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
	Material::Material()
		: description({})
	{
		gpu_data = MaterialGlobals::get()->new_shared_data();
		gpu_data_buffer_offset = MaterialGlobals::get()->get_shared_data_index(gpu_data);
		for (uint32_t i = 0; i < MAX_MATERIAL_TEXTURES; ++i)
		{
			bound_texture_handles[i] = -1;
			gpu_data->textures[i] = -1;
			gpu_data->tiling_coeffs[i] = 1;
		}
	}

	Material::~Material()
	{
		MaterialGlobals::get()->release_shared_data(gpu_data);
	}

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
						.image_filter = ImageFilter::Linear
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
		
		const uint32_t current_buffered_frame = gfx_context->get_buffered_frame_number();

		if (material_ptr->b_needs_texture_upload)
		{
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

			DescriptorHelpers::write_bindless_descriptors(gfx_context, bindless_writes, material_ptr->bound_texture_handles.data());

			for (uint32_t i = 0; i < MAX_MATERIAL_TEXTURES; ++i)
			{
				if (material_ptr->bound_texture_handles[i] >= 0)
				{
					material_ptr->gpu_data->textures[i] = (int32_t)(0x0000ffff & material_ptr->bound_texture_handles[i]);
				}
			}

			material_ptr->b_dirty = true;
			material_ptr->b_needs_texture_upload = false;
		}

		if (material_ptr->b_dirty)
		{
			// Update mapped SSBO data
			Buffer* const material_buffer = CACHE_FETCH(Buffer, MaterialGlobals::get()->material_data.data_buffer[current_buffered_frame]);
			material_buffer->copy_from(
				gfx_context,
				material_ptr->gpu_data,
				sizeof(MaterialData),
				sizeof(MaterialData) * material_ptr->gpu_data_buffer_offset
			);

			material_ptr->b_dirty = false;
		}
	}

	void material_set_texture_tiling(class GraphicsContext* const gfx_context, MaterialID material, uint32_t texture_index, float texture_tiling)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot set texture tiling for a null material!");
		assert(texture_index >= 0 && texture_index < MAX_MATERIAL_TEXTURES && "Invalid material texture index provided.");

		material_ptr->gpu_data->tiling_coeffs[texture_index] = texture_tiling;
		material_ptr->b_dirty = true;
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
		}
		return material_id;
	}
}
