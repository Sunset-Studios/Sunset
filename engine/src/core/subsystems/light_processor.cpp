#include <core/subsystems/light_processor.h>
#include <core/layers/scene.h>
#include <core/data_globals.h>
#include <core/ecs/components/light_component.h>
#include <core/ecs/components/transform_component.h>
#include <graphics/resource/buffer.h>
#include <graphics/renderer.h>

namespace Sunset
{
	void LightProcessor::initialize(class Scene* scene)
	{
		for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			if (LightGlobals::get()->light_data.data_buffer[i] == 0)
			{
				LightGlobals::get()->light_data.data_buffer[i] = BufferFactory::create(
					Renderer::get()->context(),
					{
						.name = "light_datas",
						.buffer_size = sizeof(LightData) * MIN_ENTITIES,
						.type = BufferType::StorageBuffer
					}
				);
			}
		}
	}

	void LightProcessor::update(class Scene* scene, double delta_time)
	{
		GraphicsContext* const gfx_context = Renderer::get()->context();
		const uint32_t current_buffered_frame = gfx_context->get_buffered_frame_number();

		scene->scene_data.lighting[current_buffered_frame].num_lights = 0;

		for (EntityID entity : SceneView<LightComponent, TransformComponent>(*scene))
		{
			LightComponent* const light_comp = scene->get_component<LightComponent>(entity);
			TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity);

			const int32_t entity_index = get_entity_index(entity);

			EntitySceneData& entity_data = EntityGlobals::get()->entity_data[entity_index];

			if (LightGlobals::get()->light_dirty_states.test(light_comp->light_data_buffer_offset))
			{
				const glm::vec4 light_position = transform_comp->transform.local_matrix[3];
				entity_data.bounds_pos_radius = glm::vec4(light_position.x, light_position.y, light_position.z, light_comp->light->radius);
				entity_data.local_transform = transform_comp->transform.local_matrix;
				LightGlobals::get()->light_dirty_states.unset(light_comp->light_data_buffer_offset);
			}

			scene->scene_data.lighting[current_buffered_frame].num_lights += light_comp->light->color.a > 0.0f;
		}

		QUEUE_RENDERGRAPH_COMMAND(CopyLightData, ([](class RenderGraph& render_graph, RGFrameData& frame_data, void* command_buffer)
		{
			// TODO: Only update dirtied entities instead of re-uploading the buffer every frame
			Buffer* const lights_buffer = CACHE_FETCH(Buffer, LightGlobals::get()->light_data.data_buffer[frame_data.buffered_frame_number]);
			lights_buffer->copy_from(
				frame_data.gfx_context,
				LightGlobals::get()->light_data.data.data(),
				LightGlobals::get()->light_data.data.size() * sizeof(LightData)
			);
		}));
	}
}
