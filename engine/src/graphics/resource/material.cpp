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
	void material_bind_pipeline(class GraphicsContext* const gfx_context, void* cmd_buffer, MaterialID material)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot bind pipeline for null material!");
		CACHE_FETCH(PipelineState, material_ptr->pipeline_state)->bind(gfx_context, cmd_buffer);
	}

	void material_setup_pipeline_state(class GraphicsContext* const gfx_context, MaterialID material, RenderPassID render_pass)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot setup pipeline for null material!");

		if (material_ptr->b_dirty)
		{
			const uint32_t current_buffered_frame = gfx_context->get_buffered_frame_number();

			RenderPass* const pass = CACHE_FETCH(RenderPass, render_pass);

			PipelineGraphicsStateBuilder state_builder = PipelineGraphicsStateBuilder::create_default(gfx_context->get_window()).clear_shader_stages().value();

			for (const std::pair<PipelineShaderStageType, const char*>& shader : material_ptr->description.shaders)
			{
				state_builder.set_shader_stage(shader.first, shader.second);
			}

			state_builder.derive_shader_layout();

			state_builder.set_pass(render_pass);

			material_ptr->pipeline_state = state_builder.finish();

			material_ptr->b_dirty = false;
		}
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
						.image_filter = ImageFilter::Nearest
					}
				);
			}
		}
	}

	void material_upload_textures(class GraphicsContext* const gfx_context, MaterialID material, class DescriptorSet* descriptor_set)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot load material textures for a null material!");

		const uint32_t current_buffered_frame = gfx_context->get_buffered_frame_number();

		std::vector<DescriptorBindlessWrite> bindless_writes;
		for (int i = 0; i < material_ptr->textures.size(); ++i)
		{
			const ImageID texture = material_ptr->textures[i];
			bindless_writes.push_back(
				DescriptorBindlessWrite{
					.slot = 1,
					.type = DescriptorType::Image,
					.buffer = CACHE_FETCH(Image, texture),
					.set = descriptor_set
				}
			);
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
	}

	Sunset::PipelineStateID material_get_pipeline(MaterialID material)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot get pipeline for null material!");
		return material_ptr->pipeline_state;
	}

	void material_bind_descriptors(class GraphicsContext* const gfx_context, void* cmd_buffer, MaterialID material)
	{
		Material* const material_ptr = CACHE_FETCH(Material, material);
		assert(material_ptr != nullptr && "Cannot bind descriptors for null material!");

		const ShaderLayoutID pipeline_layout = CACHE_FETCH(PipelineState, material_ptr->pipeline_state)->get_state_data().layout;

		if (material_ptr->descriptor_data.descriptor_set != nullptr)
		{
			const uint16_t material_descriptor_set_index = static_cast<uint16_t>(DescriptorSetType::Material);
			material_ptr->descriptor_data.descriptor_set->bind(gfx_context, cmd_buffer, pipeline_layout, PipelineStateType::Graphics, material_ptr->descriptor_data.dynamic_buffer_offsets, material_descriptor_set_index);
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
