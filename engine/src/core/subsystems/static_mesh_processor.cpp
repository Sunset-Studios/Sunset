#include <core/subsystems/static_mesh_processor.h>
#include <core/ecs/components/mesh_component.h>
#include <core/ecs/components/transform_component.h>
#include <core/ecs/components/camera_control_component.h>
#include <core/layers/scene.h>
#include <core/data_globals.h>
#include <graphics/resource_state.h>
#include <graphics/pipeline_state.h>
#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/mesh.h>
#include <graphics/render_pass.h>
#include <graphics/pipeline_types.h>
#include <graphics/resource/buffer.h>
#include <graphics/descriptor.h>
#include <graphics/resource/image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Sunset
{	
	void StaticMeshProcessor::initialize(class Scene* scene)
	{
		ZoneScopedN("StaticMeshProcessor::initialize");

		for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			if (MaterialGlobals::get()->material_data.data_buffer[i] == 0)
			{
				MaterialGlobals::get()->material_data.data_buffer[i] = BufferFactory::create(
					Renderer::get()->context(),
					{
						.name = "material_datas",
						.buffer_size = sizeof(MaterialData) * MAX_MATERIALS,
						.type = BufferType::StorageBuffer
					}
				);
			}
		}
	}

	void StaticMeshProcessor::update(class Scene* scene, double delta_time)
	{
		GraphicsContext* const gfx_context = Renderer::get()->context();
		const uint32_t current_buffered_frame = gfx_context->get_buffered_frame_number();

		for (EntityID entity : SceneView<MeshComponent, TransformComponent>(*scene))
		{
			const int32_t entity_index = get_entity_index(entity);

			MeshComponent* const mesh_comp = scene->get_component<MeshComponent>(entity);
			TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity);

			EntitySceneData& entity_data = EntityGlobals::get()->entity_data[entity_index];

			if (EntityGlobals::get()->entity_transform_dirty_states.test(entity_index))
			{
				const Bounds transformed_bounds = transform_mesh_bounds(mesh_comp, transform_comp->transform.local_matrix);
				entity_data.bounds_extent_and_custom_scale = glm::vec4(transformed_bounds.extents, mesh_comp->custom_bounds_scale);
				entity_data.bounds_pos_radius = glm::vec4(transformed_bounds.origin, transformed_bounds.radius);
				entity_data.local_transform = transform_comp->transform.local_matrix;
			}

			for (uint32_t section_idx = 0; section_idx < mesh_comp->section_count; ++section_idx)
			{
				if (mesh_comp->resource_states[section_idx] == 0)
				{
					mesh_comp->resource_states[section_idx]  = ResourceStateBuilder::create()
						.set_vertex_buffer(mesh_vertex_buffer(mesh_comp))
						.set_vertex_count(mesh_vertex_count(mesh_comp))
						.set_index_buffer(mesh_index_buffer(mesh_comp, section_idx))
						.set_index_count(mesh_index_count(mesh_comp, section_idx))
						.finish();
				}

				Material* const material = CACHE_FETCH(Material, mesh_comp->materials[section_idx]);
				assert(material != nullptr && "Cannot process mesh with a null material");

				Renderer::get()
					->fresh_rendertask()
					->setup(mesh_comp->materials[section_idx], mesh_comp->resource_states[section_idx], 0)
					->set_entity(entity_index)
					->set_material_index(material->gpu_data_buffer_offset[current_buffered_frame])
					->submit(Renderer::get()->get_mesh_task_queue(current_buffered_frame));
			}
		}

		// TODO: Only update dirtied entities instead of re-uploading the buffer every frame
		Buffer* const entity_buffer = CACHE_FETCH(Buffer, EntityGlobals::get()->entity_data.data_buffer[current_buffered_frame]);
		entity_buffer->copy_from(
			gfx_context,
			EntityGlobals::get()->entity_data.data.data(),
			EntityGlobals::get()->entity_data.data.size() * sizeof(EntitySceneData)
		);
	}
}
