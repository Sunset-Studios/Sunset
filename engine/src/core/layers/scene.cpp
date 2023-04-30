#include <core/layers/scene.h>
#include <core/subsystems/static_mesh_processor.h>
#include <core/subsystems/transform_processor.h>
#include <core/subsystems/camera_control_processor.h>
#include <core/subsystems/camera_input_controller.h>
#include <core/subsystems/scene_lighting_processor.h>
#include <core/subsystems/light_processor.h>
#include <core/ecs/components/camera_control_component.h>

#include <window/window.h>
#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/image.h>

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
			Window* const window = Renderer::get()->context()->get_window();

			active_camera = make_entity();
			CameraControlComponent* const camera_control_comp = assign_component<CameraControlComponent>(active_camera);
			set_fov(camera_control_comp, 75.0f);
			set_aspect_ratio(camera_control_comp, (float)window->get_extent().x / (float)window->get_extent().y);
			set_near_plane(camera_control_comp, 0.01f);
			set_far_plane(camera_control_comp, 1000.0f);
			set_position(camera_control_comp, glm::vec3(0.0f, 25.0f, 25.0f));
			set_move_speed(camera_control_comp, 50.0f);
			set_look_speed(camera_control_comp, 100.0f);
		}
	}

	void Scene::setup_subsystems()
	{
		// TODO: Potentially look for a more extensible way to add arbitrary external subsystems
		add_subsystem<CameraInputController>();
		add_subsystem<CameraControlProcessor>();
		add_subsystem<TransformProcessor>();
		add_subsystem<StaticMeshProcessor>();
		add_subsystem<LightProcessor>();
		add_subsystem<SceneLightingProcessor>();
	}

	void Scene::setup_renderer_data()
	{
		const size_t min_ubo_alignment = Renderer::get()->context()->get_min_ubo_offset_alignment();
		const size_t aligned_cam_data_size = BufferHelpers::pad_ubo_size(sizeof(CameraData), min_ubo_alignment);
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

		const ImageID default_image = ImageFactory::create_default(Renderer::get()->context());

		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			const uint32_t cam_data_buffer_offset = static_cast<uint32_t>(scene_data.cam_data_buffer_start + aligned_cam_data_size * i);
			const uint32_t lighting_data_buffer_offset = static_cast<uint32_t>(scene_data.lighting_data_buffer_start + aligned_lighting_data_size * i);
			const BufferID buffer = scene_data.buffer;

			Renderer::get()->get_render_graph().queue_global_descriptor_writes(Renderer::get()->context(), i,
			{
				{
					.buffer = CACHE_FETCH(Buffer, buffer)->get(),
					.buffer_range = sizeof(CameraData),
					.buffer_offset = cam_data_buffer_offset
				},
				{
					.buffer = CACHE_FETCH(Buffer, buffer)->get(),
					.buffer_range = sizeof(SceneLightingData),
					.buffer_offset = lighting_data_buffer_offset
				},
				{
					.buffer = CACHE_FETCH(Image, default_image)
				},
				{
					.buffer = CACHE_FETCH(Image, default_image)
				}
			});
		}
	}

	void set_scene_fog_color(class Scene* scene, glm::vec4 fog_color)
	{
		assert(scene != nullptr && "Cannot set fog color on null scene!");
		scene->scene_data.lighting.fog_color = fog_color;
	}

	void set_scene_fog_distance(class Scene* scene, float fog_distance)
	{
		assert(scene != nullptr && "Cannot set fog distance on null scene!");
		scene->scene_data.lighting.fog_distance = fog_distance;
	}

	void set_scene_sunlight_direction(class Scene* scene, glm::vec4 sunlight_direction)
	{
		assert(scene != nullptr && "Cannot set sunlight direction on null scene!");
		scene->scene_data.lighting.sunlight_direction = sunlight_direction;
	}

	void set_scene_sunlight_color(class Scene* scene, glm::vec4 sunlight_color)
	{
		assert(scene != nullptr && "Cannot set sunlight color on null scene!");
		scene->scene_data.lighting.sunlight_color = sunlight_color;
	}

	void set_scene_sunlight_intensity(class Scene* scene, float sunlight_intensity)
	{
		assert(scene != nullptr && "Cannot set sunlight intensity on null scene!");
		scene->scene_data.lighting.sunlight_intensity = sunlight_intensity;
	}

	void set_scene_sunlight_angular_radius(class Scene* scene, float angular_radius)
	{
		assert(scene != nullptr && "Cannot set sunlight angular radius on null scene!");
		scene->scene_data.lighting.sunlight_angular_radius = angular_radius;
	}

	void set_scene_atmospheric_turbidity(class Scene* scene, float turbidity)
	{
		assert(scene != nullptr && "Cannot set atmospheric turbidity on null scene!");
		scene->scene_data.lighting.atmospheric_turbidity = turbidity;
	}

	void set_scene_atmospheric_rayleigh(class Scene* scene, float rayleigh)
	{
		assert(scene != nullptr && "Cannot set atmospheric rayleigh on null scene!");
		scene->scene_data.lighting.atmospheric_rayleigh = rayleigh;
	}

	void set_scene_mie_coefficient(class Scene* scene, float coeff)
	{
		assert(scene != nullptr && "Cannot set mie coefficient on null scene!");
		scene->scene_data.lighting.mie_coefficient = coeff;
	}

	void set_scene_mie_directional_g(class Scene* scene, float g)
	{
		assert(scene != nullptr && "Cannot set mie directional G on null scene!");
		scene->scene_data.lighting.mie_directional_g = g;
	}
}
