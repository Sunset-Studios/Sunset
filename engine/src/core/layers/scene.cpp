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
		add_subsystem<CameraControlProcessor>();
		add_subsystem<CameraInputController>();
		add_subsystem<TransformProcessor>();
		add_subsystem<StaticMeshProcessor>();
		add_subsystem<SceneLightingProcessor>();
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

	void Scene::setup_renderer_data()
	{
		const size_t min_ubo_alignment = Renderer::get()->context()->get_min_ubo_offset_alignment();

		if (scene_lighting.buffer == nullptr)
		{
			const size_t scene_lighting_buffer_size = MAX_BUFFERED_FRAMES * BufferHelpers::pad_ubo_size(sizeof(SceneLightingData), min_ubo_alignment);
			scene_lighting.buffer = BufferFactory::create(Renderer::get()->context(), scene_lighting_buffer_size, BufferType::UniformBuffer);
		}

		for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			if (CameraControlComponent::gpu_cam_buffers[i] == nullptr)
			{
				CameraControlComponent::gpu_cam_buffers[i] = BufferFactory::create(Renderer::get()->context(), sizeof(CameraMatrices), BufferType::UniformBuffer);
			}

			Renderer::get()->inject_global_descriptor(i,
			{
				{.binding = 0, .buffer = CameraControlComponent::gpu_cam_buffers[i], .buffer_offset = 0, .buffer_range = sizeof(CameraMatrices), .type = DescriptorType::UniformBuffer, .shader_stages = PipelineShaderStageType::Vertex },
				{.binding = 1, .buffer = scene_lighting.buffer, .buffer_offset = static_cast<uint32_t>(BufferHelpers::pad_ubo_size(sizeof(SceneLightingData), min_ubo_alignment) * i), .buffer_range = sizeof(SceneLightingData), .type = DescriptorType::DynamicUniformBuffer, .shader_stages = PipelineShaderStageType::Vertex | PipelineShaderStageType::Fragment }
			});
		}
	}
}
