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
			if (frame_data.global_descriptor_set != nullptr && scene->scene_data.lighting.irradiance_map == -1 && scene->scene_data.irradiance_map != 0)
			{
				std::vector<DescriptorBindlessWrite> bindless_writes;
				bindless_writes.push_back(
					DescriptorBindlessWrite{
						.slot = ImageBindTableSlot,
						.type = DescriptorType::Image,
						.buffer = CACHE_FETCH(Image, scene->scene_data.irradiance_map),
						.set = frame_data.global_descriptor_set
					}
				);

				BindingTableHandle bound_texture_handle;
				DescriptorHelpers::write_bindless_descriptors(Renderer::get()->context(), bindless_writes, &bound_texture_handle);

				if (bound_texture_handle >= 0)
				{
					scene->scene_data.lighting.irradiance_map = (0x0000ffff & bound_texture_handle);
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
