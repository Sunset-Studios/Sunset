#include <core/subsystems/scene_lighting_processor.h>
#include <core/layers/scene.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/image.h>
#include <graphics/renderer.h>
#include <graphics/light.h>
#include <graphics/descriptor.h>

namespace Sunset
{
	void SceneLightingProcessor::initialize(class Scene* scene)
	{

	}

	void SceneLightingProcessor::update(class Scene* scene, double delta_time)
	{
		const size_t min_ubo_alignment = Renderer::get()->context()->get_min_ubo_offset_alignment();
		const uint32_t buffered_frame = Renderer::get()->context()->get_buffered_frame_number();

		QUEUE_RENDERGRAPH_COMMAND(SetIrradianceMap, [=](class RenderGraph& render_graph, RGFrameData& frame_data, void* command_buffer)
		{
			auto write_bindless_lighting_image = [=](ImageID lighting_image, int32_t& bindless_index)
			{
				std::vector<DescriptorBindlessWrite> bindless_writes;
				bindless_writes.push_back(
					DescriptorBindlessWrite{
						.slot = ImageBindTableSlot,
						.type = DescriptorType::Image,
						.buffer = CACHE_FETCH(Image, lighting_image),
						.set = frame_data.global_descriptor_set
					}
				);

				BindingTableHandle bound_texture_handle;
				DescriptorHelpers::write_bindless_descriptors(Renderer::get()->context(), bindless_writes, &bound_texture_handle);

				if (bound_texture_handle >= 0)
				{
					bindless_index = (0x0000ffff & bound_texture_handle);
				}
			};

			// Push IBL textures to descriptor set
			if (frame_data.global_descriptor_set != nullptr)
			{
				if (scene->scene_data.lighting.irradiance_map == -1 && scene->scene_data.irradiance_map != 0)
				{
					write_bindless_lighting_image(scene->scene_data.irradiance_map, scene->scene_data.lighting.irradiance_map);
				}
				if (scene->scene_data.lighting.sky_box == -1 && scene->scene_data.sky_box != 0)
				{
					write_bindless_lighting_image(scene->scene_data.sky_box, scene->scene_data.lighting.sky_box);
				}
				if (scene->scene_data.lighting.prefilter_map == -1 && scene->scene_data.prefilter_map != 0)
				{
					write_bindless_lighting_image(scene->scene_data.prefilter_map, scene->scene_data.lighting.prefilter_map);
				}
				if (scene->scene_data.lighting.brdf_lut == -1 && scene->scene_data.brdf_lut != 0)
				{
					write_bindless_lighting_image(scene->scene_data.brdf_lut, scene->scene_data.lighting.brdf_lut);
				}
			}

			CACHE_FETCH(Buffer, scene->scene_data.buffer)->copy_from(
				Renderer::get()->context(),
				&scene->scene_data.lighting,
				sizeof(SceneLightingData),
				scene->scene_data.lighting_data_buffer_start + BufferHelpers::pad_ubo_size(sizeof(SceneLightingData), min_ubo_alignment) * buffered_frame
			);
		});
	}
}
