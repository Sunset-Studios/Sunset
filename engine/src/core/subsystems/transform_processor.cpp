#include <core/subsystems/transform_processor.h>
#include <core/ecs/components/transform_component.h>
#include <core/layers/scene.h>
#include <graphics/renderer.h>
#include <core/data_globals.h>
#include <graphics/resource/buffer.h>

namespace Sunset
{
	void TransformProcessor::initialize(class Scene* scene)
	{
		for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			if (EntityGlobals::get()->entity_data.data_buffer[i] == 0)
			{
				EntityGlobals::get()->entity_data.data_buffer[i] = BufferFactory::create(
					Renderer::get()->context(),
					{
						.name = "entity_datas",
						.buffer_size = sizeof(EntitySceneData) * MIN_ENTITIES,
						.type = BufferType::StorageBuffer
					}
				);
			}
		}
	}

	void TransformProcessor::update(class Scene* scene, double delta_time)
	{
		for (EntityID entity : SceneView<TransformComponent>(*scene))
		{
			TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity);

			if (transform_comp->transform.b_dirty)
			{
				recalculate_transform(transform_comp);
				transform_comp->transform.b_dirty = false;

				EntityGlobals::get()->entity_transform_dirty_states.set(get_entity_index(entity));
			}
		}
	}

	void TransformProcessor::post_update(Scene* scene)
	{
		EntityGlobals::get()->entity_transform_dirty_states.reset();
	}
}
