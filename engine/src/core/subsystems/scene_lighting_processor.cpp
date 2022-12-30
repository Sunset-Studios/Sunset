#include <core/subsystems/scene_lighting_processor.h>
#include <core/layers/scene.h>
#include <graphics/resource/buffer.h>
#include <graphics/renderer.h>

namespace Sunset
{
	void SceneLightingProcessor::update(class Scene* scene, double delta_time)
	{
		const uint16_t buffered_frame = Renderer::get()->context()->get_buffered_frame_number();
		const float framed = Renderer::get()->context()->get_frame_number() / 120.0f;
		scene->scene_lighting.data.ambient_color = { glm::sin(framed), 0.0f, glm::cos(framed), 1.0f };

		scene->scene_lighting.buffer->copy_from(
			Renderer::get()->context(),
			&scene->scene_lighting.data,
			sizeof(SceneLightingData),
			BufferHelpers::pad_ubo_size(sizeof(SceneLightingData), Renderer::get()->context()->get_min_ubo_offset_alignment()) * buffered_frame
		);
	}
}
