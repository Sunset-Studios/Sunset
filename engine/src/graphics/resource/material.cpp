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
		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			gpu_data[i] = MaterialGlobals::get()->new_shared_data();
			gpu_data_buffer_offset[i] = MaterialGlobals::get()->get_shared_data_index(gpu_data[i]);
			for (uint32_t j = 0; j < MAX_MATERIAL_TEXTURES; ++j)
			{
				bound_texture_handles[MAX_MATERIAL_TEXTURES * i + j] = -1;
				gpu_data[i]->textures[j] = -1;
				gpu_data[i]->tiling_coeffs[j] = 1;
			}
			b_needs_texture_upload[i] = false;
			b_dirty[i] = true;
		}
	}

	Material::~Material()
	{
		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			MaterialGlobals::get()->release_shared_data(gpu_data[i]);
		}
	}

	void material_load_textures(class GraphicsContext* const gfx_context, MaterialID material)
	{
		ZoneScopedN("material_load_textures");

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
						.image_filter = ImageFilter::Linear,
						.linear_mip_filtering = true
					}
				);
				for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
				{
					material_ptr->b_needs_texture_upload[i] = true;
				}
			}
		}
	}

	void material_set_gpu_params(class GraphicsContext* const gfx_context, MaterialID material)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot set texture tiling for a null material!");

		material_set_color(gfx_context, material, material_ptr->description.color);
		material_set_uniform_roughness(gfx_context, material, material_ptr->description.uniform_roughness);
		material_set_uniform_metallic(gfx_context, material, material_ptr->description.uniform_metallic);
		material_set_uniform_reflectance(gfx_context, material, material_ptr->description.uniform_reflectance);
		material_set_uniform_clearcoat(gfx_context, material, material_ptr->description.uniform_clearcoat);
		material_set_uniform_clearcoat_roughness(gfx_context, material, material_ptr->description.uniform_clearcoat_roughness);
		material_set_uniform_emissive(gfx_context, material, material_ptr->description.uniform_emissive);
	}

	void material_update(class GraphicsContext* const gfx_context, MaterialID material, class DescriptorSet* descriptor_set, int32_t buffered_frame_number)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot load material textures for a null material!");
		
		if (material_ptr->b_needs_texture_upload[buffered_frame_number])
		{
			ZoneScopedN("material_update: Upload textures");

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

			const uint32_t bound_texture_handles_offset = buffered_frame_number * MAX_MATERIAL_TEXTURES;

			DescriptorHelpers::write_bindless_descriptors(gfx_context, bindless_writes, material_ptr->bound_texture_handles.data() + bound_texture_handles_offset);

			for (uint32_t i = 0; i < MAX_MATERIAL_TEXTURES; ++i)
			{
				const uint32_t bound_texture_handle_idx = bound_texture_handles_offset + i;
				if (material_ptr->bound_texture_handles[bound_texture_handle_idx] >= 0)
				{
					material_ptr->gpu_data[buffered_frame_number]->textures[i] = (int32_t)(0x0000ffff & material_ptr->bound_texture_handles[bound_texture_handle_idx]);
				}
			}

			material_ptr->b_dirty[buffered_frame_number] = true;
			material_ptr->b_needs_texture_upload[buffered_frame_number] = false;
		}

		if (material_ptr->b_dirty[buffered_frame_number])
		{
			// Update mapped SSBO data
			Buffer* const material_buffer = CACHE_FETCH(Buffer, MaterialGlobals::get()->material_data.data_buffer[buffered_frame_number]);
			material_buffer->copy_from(
				gfx_context,
				material_ptr->gpu_data[buffered_frame_number],
				sizeof(MaterialData),
				sizeof(MaterialData) * material_ptr->gpu_data_buffer_offset[buffered_frame_number]
			);

			material_ptr->b_dirty[buffered_frame_number] = false;
		}
	}

	void material_set_texture_tiling(class GraphicsContext* const gfx_context, MaterialID material, uint32_t texture_index, float texture_tiling)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot set texture tiling for a null material!");
		assert(texture_index >= 0 && texture_index < MAX_MATERIAL_TEXTURES && "Invalid material texture index provided.");

		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			material_ptr->gpu_data[i]->tiling_coeffs[texture_index] = texture_tiling;
			material_ptr->b_dirty[i] = true;
		}
	}

	void material_set_color(class GraphicsContext* const gfx_context, MaterialID material, glm::vec3 color)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot set texture tiling for a null material!");

		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			material_ptr->gpu_data[i]->color = color;
			material_ptr->b_dirty[i] = true;
		}
	}

	void material_set_uniform_roughness(class GraphicsContext* const gfx_context, MaterialID material, float roughness)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot set texture tiling for a null material!");

		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			material_ptr->gpu_data[i]->uniform_roughness = roughness;
			material_ptr->b_dirty[i] = true;
		}
	}

	void material_set_uniform_metallic(class GraphicsContext* const gfx_context, MaterialID material, float metallic)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot set texture tiling for a null material!");

		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			material_ptr->gpu_data[i]->uniform_metallic = metallic;
			material_ptr->b_dirty[i] = true;
		}
	}

	void material_set_uniform_reflectance(class GraphicsContext* const gfx_context, MaterialID material, float reflectance)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot set texture tiling for a null material!");

		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			material_ptr->gpu_data[i]->uniform_reflectance = reflectance;
			material_ptr->b_dirty[i] = true;
		}
	}

	void material_set_uniform_clearcoat(class GraphicsContext* const gfx_context, MaterialID material, float clearcoat)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot set texture tiling for a null material!");

		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			material_ptr->gpu_data[i]->uniform_clearcoat = clearcoat;
			material_ptr->b_dirty[i] = true;
		}
	}

	void material_set_uniform_clearcoat_roughness(class GraphicsContext* const gfx_context, MaterialID material, float clearcoat_roughness)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot set texture tiling for a null material!");

		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			material_ptr->gpu_data[i]->uniform_clearcoat_roughness = clearcoat_roughness;
			material_ptr->b_dirty[i] = true;
		}
	}

	void material_set_uniform_emissive(GraphicsContext* const gfx_context, MaterialID material, float emissive)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot set texture tiling for a null material!");

		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			material_ptr->gpu_data[i]->uniform_emissive = emissive;
			material_ptr->b_dirty[i] = true;
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
			material_set_gpu_params(gfx_context, material_id);
		}
		return material_id;
	}
}
