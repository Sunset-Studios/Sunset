#include <core/layers/scene.h>
#include <core/subsystems/static_mesh_processor.h>
#include <core/subsystems/transform_processor.h>
#include <core/subsystems/camera_control_processor.h>
#include <core/subsystems/camera_input_controller.h>
#include <core/subsystems/scene_lighting_processor.h>
#include <core/ecs/components/camera_control_component.h>

#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>

namespace Sunset
{
	Scene::Scene()
	{
		subsystems.reserve(MAX_COMPONENTS);
		component_pools.reserve(MAX_COMPONENTS);
		entities.reserve(MIN_ENTITIES);
		free_entities.reserve(MIN_ENTITIES);
	}

	void Scene::initialize()
	{
		add_default_camera();
		setup_subsystems();
		setup_renderer_data();
	}

	void Scene::destroy()
	{
		for (auto it = subsystems.begin(); it != subsystems.end(); ++it)
		{
			(*it)->destroy(this);
		}

		subsystems.clear();
		component_pools.clear();
		entities.clear();;
		free_entities.clear();
	}

	void Scene::update(double delta_time)
	{
		for (auto it = subsystems.begin(); it != subsystems.end(); ++it)
		{
			(*it)->update(this, delta_time);
		}
	}

	Sunset::EntityID Scene::make_entity()
	{
		if (!free_entities.empty())
		{
			const EntityIndex new_index = free_entities.back();
			free_entities.pop_back();
			const EntityID new_id = create_entity_id(new_index, get_entity_version(entities[new_index].id));
			entities[new_index].id = new_id;
			return entities[new_index].id;
		}
		entities.push_back({ create_entity_id(EntityIndex(entities.size()), 0), ComponentMask() });
		return entities.back().id;
	}

	void Scene::destroy_entity(EntityID entity_id)
	{
		const EntityID new_id = create_entity_id(EntityIndex(-1), get_entity_version(entity_id) + 1);
		entities[get_entity_index(entity_id)].id = new_id;
		entities[get_entity_index(entity_id)].components.reset();
		free_entities.push_back(get_entity_index(entity_id));
	}

	void Scene::add_default_camera()
	{
		if (active_camera == 0)
		{
			active_camera = make_entity();
			CameraControlComponent* const camera_control_comp = assign_component<CameraControlComponent>(active_camera);
			set_fov(camera_control_comp, 90.0f);
			set_aspect_ratio(camera_control_comp, 1280.0f / 720.0f);
			set_near_plane(camera_control_comp, 0.1f);
			set_far_plane(camera_control_comp, 200.0f);
			set_position(camera_control_comp, glm::vec3(0.0f, 0.0f, 0.0f));
			set_move_speed(camera_control_comp, 1.0f);
			set_look_speed(camera_control_comp, 0.1f);
		}
	}

	void Scene::setup_subsystems()
	{
		// TODO: Potentially look for a more extensible way to add arbitrary external subsystems
		add_subsystem<CameraControlProcessor>();
		add_subsystem<CameraInputController>();
		add_subsystem<TransformProcessor>();
		add_subsystem<StaticMeshProcessor>();
		add_subsystem<SceneLightingProcessor>();
	}

	void Scene::setup_renderer_data()
	{
		const size_t min_ubo_alignment = Renderer::get()->context()->get_min_ubo_offset_alignment();
		const size_t aligned_cam_data_size = BufferHelpers::pad_ubo_size(sizeof(CameraMatrices), min_ubo_alignment);
		const size_t aligned_lighting_data_size = BufferHelpers::pad_ubo_size(sizeof(SceneLightingData), min_ubo_alignment);

		scene_data.cam_data_buffer_start = 0;
		scene_data.lighting_data_buffer_start = aligned_cam_data_size * MAX_BUFFERED_FRAMES;

		if (scene_data.buffer == 0)
		{
			scene_data.buffer = BufferFactory::create(
				Renderer::get()->context(),
				{
					.name = "scene_data_buffer",
					.buffer_size = (aligned_cam_data_size + aligned_lighting_data_size) * MAX_BUFFERED_FRAMES,
					.type = BufferType::UniformBuffer
				}
			);
		}

		for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			Renderer::get()->get_render_graph().inject_global_descriptors(Renderer::get()->context(), i,
			{
				{
					.binding = 0,
					.buffer = scene_data.buffer,
					.buffer_offset = static_cast<uint32_t>(scene_data.cam_data_buffer_start + aligned_cam_data_size * i),
					.buffer_range = sizeof(CameraMatrices),
					.count = 1,
					.type = DescriptorType::DynamicUniformBuffer,
					.shader_stages = PipelineShaderStageType::All
				},
				{
					.binding = 1,
					.buffer = scene_data.buffer,
					.buffer_offset = static_cast<uint32_t>(scene_data.lighting_data_buffer_start + aligned_lighting_data_size * i),
					.buffer_range = sizeof(SceneLightingData),
					.count = 1,
					.type = DescriptorType::DynamicUniformBuffer,
					.shader_stages = PipelineShaderStageType::All
				}
			});
		}
	}
}
