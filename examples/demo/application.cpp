#include <application.h>
#include <window/window.h>
#include <graphics/renderer.h>
#include <graphics/strategies/deferred_shading.h>
#include <graphics/strategies/forward_shading.h>
#include <input/input_provider.h>
#include <core/simulation_core.h>
#include <core/layers/scene.h>
#include <core/layers/editor_gui.h>
#include <graphics/resource/mesh.h>
#include <utility/cvar.h>

#include <core/ecs/components/mesh_component.h>
#include <core/ecs/components/transform_component.h>
#include <core/ecs/components/light_component.h>
#include <core/ecs/components/body_component.h>
#include <core/ecs/components/camera_control_component.h>

#include <player/paddle_controller.h>

#include <SDL.h>
#include <SDL_vulkan.h>

#include <tracy/Tracy.hpp>

namespace Sunset
{
	void Application::init()
	{
		window = WindowFactory::create("Breaker", glm::ivec2(0), glm::ivec2(1920, 1080), false);
		window->capture_mouse(true);

		Renderer::get()->setup(window);

		InputProvider::get()->push_context(InputProvider::default_context());

		Physics::get()->context()->set_global_gravity(glm::vec3(0.0f, 0.0f, 0.0f));

		// Adding our scene objects programmatically
		{
			std::unique_ptr<Scene> scene = std::make_unique<Scene>();

			scene->add_subsystem<PaddleController>();

			set_scene_sunlight_intensity(scene.get(), 1.0f);
			set_scene_sunlight_angular_radius(scene.get(), 0.9999566769f);
			set_scene_sunlight_direction(scene.get(), glm::normalize(glm::vec3(1.0f, 0.5f, -0.25f)));
			set_scene_atmospheric_turbidity(scene.get(), 2.542f);
			set_scene_atmospheric_rayleigh(scene.get(), 1.0f);
			set_scene_mie_coefficient(scene.get(), 0.005f);
			set_scene_mie_directional_g(scene.get(), 0.8f);

			set_scene_sky_irradiance(scene.get(), "../../assets/sky/irradiance/sky_irradiance");
			set_scene_prefilter_map(scene.get(), "../../assets/sky/prefilter/sky_prefilter");
			set_scene_brdf_lut(scene.get(), "../../assets/sky/brdf_lut/sky_brdf_lut_mip_0_layer_0.sun");

			load_scene_objects(scene.get());

			SimulationCore::get()->register_layer(std::move(scene));
		}
	}

	void Application::cleanup()
	{	
		Renderer::get()->destroy();

		SimulationCore::get()->destroy();

		window->destroy();
	}

	void Application::run()
	{
		bool bQuit = false;

		while (!bQuit && !window->is_closing())
		{
			window->poll();

			{
				ScopedRender<DeferredShadingStrategy> scoped_render(Renderer::get());

				InputProvider::get()->update(window);

				SimulationCore::get()->update();
			}

			FrameMark;
		}
	}
	
	void Application::load_scene_objects(Scene* const scene)
	{
		CameraControlComponent* camera_control_comp{ nullptr };

		// Add a camera
		{
			const glm::ivec2 res = Renderer::get()->context()->get_surface_resolution();

			scene->active_camera = scene->make_entity();
			camera_control_comp = scene->assign_component<CameraControlComponent>(scene->active_camera);
			set_fov(camera_control_comp, 75.0f);
			set_aspect_ratio(camera_control_comp, (float)res.x / (float)res.y);
			set_near_plane(camera_control_comp, 0.001f);
			set_far_plane(camera_control_comp, 1000.0f);
			set_position(camera_control_comp, glm::vec3(50.0f, 0.0f, 0.0f));
			set_forward(camera_control_comp, glm::vec3(-1.0f, 0.0f, 0.0f), true);
		}

		// Add directional light
		{
			EntityID light_entity = scene->make_entity();

			TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(light_entity);

			LightComponent* const light_comp = scene->assign_component<LightComponent>(light_entity);

			set_light_color(light_comp, glm::vec3(1.0f, 1.0f, 1.0f));
			set_light_type(light_comp, LightType::Directional);
			set_light_direction(light_comp, glm::vec3(1.0f, 1.0f, 0.0f));
			set_light_intensity(light_comp, 4.0f);
			set_light_entity_index(light_comp, get_entity_index(light_entity));
			set_light_should_use_sun_direction(light_comp, true);
			set_light_is_csm_caster(light_comp, true);
			set_light_casts_shadows(light_comp, true);
		}

		// Add a backdrop 
		{
			EntityID backdrop_ent = scene->make_entity();

			TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(backdrop_ent);

			set_rotation(transform_comp, glm::vec3(glm::radians(0.0f), glm::radians(-90.0f), glm::radians(0.0f)));
			set_scale(transform_comp, glm::vec3(100.0f, 100.0f, 100.0f));

			MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(backdrop_ent);

			MaterialID mesh_material = MaterialFactory::create(
				Renderer::get()->context(),
				{
					.textures =
					{
						"../../assets/alien-metal/alien-metal-albedo.sun",
						"../../assets/alien-metal/alien-metal-normal.sun",
						"../../assets/alien-metal/alien-metal-roughness.sun",
						"../../assets/alien-metal/alien-metal-metallic.sun",
						"../../assets/alien-metal/alien-metal-ao.sun"
					}
				}
			);

			material_set_texture_tiling(Renderer::get()->context(), mesh_material, 0, 10.0f);
			material_set_texture_tiling(Renderer::get()->context(), mesh_material, 1, 10.0f);
			material_set_texture_tiling(Renderer::get()->context(), mesh_material, 2, 10.0f);
			material_set_texture_tiling(Renderer::get()->context(), mesh_material, 3, 10.0f);
			material_set_texture_tiling(Renderer::get()->context(), mesh_material, 4, 10.0f);

			set_mesh(mesh_comp, MeshFactory::create_quad(Renderer::get()->context()));
			set_material(mesh_comp, mesh_material);
		}

		// Add bricks
		{
			EntityID brick_ent = scene->make_entity();

			TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(brick_ent);

			set_position(transform_comp, glm::vec3(25.0f, 15.0f, 15.0f * camera_control_comp->data.aspect_ratio));
			set_scale(transform_comp, glm::vec3(0.5f, 0.95f, 5.0f));

			MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(brick_ent);

			MaterialID metal_material = MaterialFactory::create(
				Renderer::get()->context(),
				{
					.color = glm::vec3(1.0f, 0.0f, 0.0f),
					.uniform_roughness = 0.2f,
					.uniform_metallic = 1.0f
				}
			);

			set_mesh(mesh_comp, MeshFactory::create_cube(Renderer::get()->context()));
			set_material(mesh_comp, metal_material);
			set_custom_bounds_scale(mesh_comp, 2.0f);

			BodyComponent* const body_comp = scene->assign_component<BodyComponent>(brick_ent);

			BoxShapeDescription box_shape
			{
				.half_extent = glm::vec3(1.0f)
			};
			set_body_shape(body_comp, box_shape);
			set_body_type(body_comp, PhysicsBodyType::Static);
			set_body_gravity_scale(body_comp, 1.0f);
			set_body_restitution(body_comp, 0.5f);
		}

		// Add player ball
		{
			EntityID ball_ent = scene->make_entity();

			TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(ball_ent);

			set_position(transform_comp, glm::vec3(25.0f, -15.0f, 0.0f));
			set_scale(transform_comp, glm::vec3(0.5f));

			MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(ball_ent);

			MaterialID drone_material = MaterialFactory::create(
				Renderer::get()->context(),
				{
					.uniform_emissive{ 25.0f },
					.textures =
					{
						"../../assets/metal-albedo.sun",
						"../../assets/metal-normal.sun",
						"../../assets/metal-roughness.sun",
						"../../assets/metal-metallic.sun",
						"../../assets/metal-ao.sun",
						"../../assets/default_black.sun"
					}
				}
			);

			set_mesh(mesh_comp, MeshFactory::create_sphere(Renderer::get()->context(), glm::ivec2(32, 32), 1.0f));
			set_material(mesh_comp, drone_material);
			set_custom_bounds_scale(mesh_comp, 2.0f);

			BodyComponent* const body_comp = scene->assign_component<BodyComponent>(ball_ent);

			SphereShapeDescription sphere_shape
			{
				.radius = 1.0f
			};
			set_body_shape(body_comp, sphere_shape);
			set_body_type(body_comp, PhysicsBodyType::Dynamic);
			set_body_gravity_scale(body_comp, 1.0f);
			set_body_restitution(body_comp, 0.5f);
		}

		// Add player paddle
		{
			EntityID paddle_ent = scene->make_entity();

			TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(paddle_ent);

			set_position(transform_comp, glm::vec3(25.0f, -17.0f, 0.0f));
			set_scale(transform_comp, glm::vec3(0.5f, 0.65f, 6.0f));

			MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(paddle_ent);

			MaterialID metal_material = MaterialFactory::create(
				Renderer::get()->context(),
				{
					.color = glm::vec3(1.0f, 1.0f, 1.0f),
					.uniform_roughness = 0.2f,
					.uniform_metallic = 1.0f
				}
			);

			set_mesh(mesh_comp, MeshFactory::create_cube(Renderer::get()->context()));
			set_material(mesh_comp, metal_material);
			set_custom_bounds_scale(mesh_comp, 2.0f);

			BodyComponent* const body_comp = scene->assign_component<BodyComponent>(paddle_ent);

			BoxShapeDescription box_shape
			{
				.half_extent = glm::vec3(1.0f)
			};
			set_body_shape(body_comp, box_shape);
			set_body_type(body_comp, PhysicsBodyType::Dynamic);
			set_body_gravity_scale(body_comp, 1.0f);
			set_body_restitution(body_comp, 0.5f);

			PaddleComponent* const paddle_comp = scene->assign_component<PaddleComponent>(paddle_ent);
		
			set_paddle_speed(paddle_comp, 10000.0f);

			scene->get_subsystem<PaddleController>()->set_paddle(paddle_ent);
		}
	}
}

