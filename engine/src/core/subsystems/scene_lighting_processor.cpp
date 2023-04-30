#include <core/subsystems/scene_lighting_processor.h>
#include <core/layers/scene.h>
#include <graphics/resource/buffer.h>
#include <graphics/renderer.h>
#include <graphics/light.h>

namespace Sunset
{
	void SceneLightingProcessor::initialize(class Scene* scene)
	{
	}

	void SceneLightingProcessor::update(class Scene* scene, double delta_time)
	{
		const size_t min_ubo_alignment = Renderer::get()->context()->get_min_ubo_offset_alignment();
		const uint32_t buffered_frame = Renderer::get()->context()->get_buffered_frame_number();

		CACHE_FETCH(Buffer, scene->scene_data.buffer)->copy_from(
			Renderer::get()->context(),
			&scene->scene_data.lighting,
			sizeof(SceneLightingData),
			scene->scene_data.lighting_data_buffer_start + BufferHelpers::pad_ubo_size(sizeof(SceneLightingData), min_ubo_alignment) * buffered_frame
		);
	}
}
