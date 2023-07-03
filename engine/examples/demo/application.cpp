﻿#include <application.h>
#include <window/window.h>
#include <graphics/renderer.h>
#include <graphics/strategies/deferred_shading.h>
#include <graphics/strategies/forward_shading.h>
#include <input/input_provider.h>
#include <core/simulation_core.h>
#include <core/layers/scene.h>
#include <core/layers/editor_gui.h>
#include <graphics/resource/mesh.h>

#include <core/ecs/components/mesh_component.h>
#include <core/ecs/components/transform_component.h>
#include <core/ecs/components/light_component.h>
#include <core/ecs/components/body_component.h>

#include <SDL.h>
#include <SDL_vulkan.h>

#include <tracy/Tracy.hpp>

namespace Sunset
{
	void Application::init()
	{
		bIsInitialized = true;

		window = WindowFactory::create(ENGINE_NAME, glm::ivec2(0), glm::ivec2(1280, 720), false);

		Renderer::get()->setup(window);

		InputProvider::get()->push_context(InputProvider::default_context());

		Physics::get()->context()->set_global_gravity(glm::vec3(0.0f, -9.81f, 0.0f));

#ifndef NDEBUG
		{
			std::unique_ptr<EditorGui> gui = std::make_unique<EditorGui>();
			SimulationCore::get()->register_layer(std::move(gui));
		}
#endif

		// Test scene
		{
			std::unique_ptr<Scene> scene = std::make_unique<Scene>();

			set_scene_sunlight_intensity(scene.get(), 1.0f);
			set_scene_sunlight_angular_radius(scene.get(), 0.9999566769f);
			set_scene_sunlight_direction(scene.get(), glm::vec4(1.0f, 0.75f, 0.25f, 0.0f));
			set_scene_atmospheric_turbidity(scene.get(), 2.542f);
			set_scene_atmospheric_rayleigh(scene.get(), 1.0f);
			set_scene_mie_coefficient(scene.get(), 0.005f);
			set_scene_mie_directional_g(scene.get(), 0.8f);

			set_scene_sky_irradiance(scene.get(), "../../assets/sky/irradiance/sky_irradiance");
			set_scene_sky_box(scene.get(), "../../assets/sky/cubemap/sky_cubemap");
			set_scene_prefilter_map(scene.get(), "../../assets/sky/prefilter/sky_prefilter");
			set_scene_brdf_lut(scene.get(), "../../assets/sky/brdf_lut/sky_brdf_lut_mip_0_layer_0.sun");

			// Add directional light
			{
				EntityID light_entity = scene->make_entity();

				TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(light_entity);

				LightComponent* const light_comp = scene->assign_component<LightComponent>(light_entity);

				set_light_color(light_comp, glm::vec3(1.0f, 1.0f, 1.0f));
				set_light_type(light_comp, LightType::Directional);
				set_light_intensity(light_comp, 4.0f);
				set_light_entity_index(light_comp, get_entity_index(light_entity));
				set_light_should_use_sun_direction(light_comp, true);
			}

			// Add point light
			{
				EntityID light_entity = scene->make_entity();

				TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(light_entity);

				set_position(transform_comp, glm::vec3(-75.0f, 25.0f, 0.0f));

				LightComponent* const light_comp = scene->assign_component<LightComponent>(light_entity);

				set_light_color(light_comp, glm::vec3(1.0f, 1.0f, 1.0f));
				set_light_type(light_comp, LightType::Point);
				set_light_radius(light_comp, 75.0f);
				set_light_intensity(light_comp, 100.0f);
				set_light_entity_index(light_comp, get_entity_index(light_entity));
			}

			// Add ground plane
			{
				EntityID mesh_ent = scene->make_entity();

				TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(mesh_ent);

				set_rotation(transform_comp, glm::vec3(glm::radians(90.0f), glm::radians(0.0f), glm::radians(0.0f)));
				set_scale(transform_comp, glm::vec3(500.0f, 500.0f, 500.0f));

				MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(mesh_ent);

				MaterialID mesh_material = MaterialFactory::create(
					Renderer::get()->context(),
					{
						.textures =
						{
							"../../assets/foam-grip-albedo.sun",
							"../../assets/foam-grip-normal.sun",
							"../../assets/foam-grip-roughness.sun",
							"../../assets/foam-grip-metallic.sun",
							"../../assets/foam-grip-ao.sun"
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

				BodyComponent* const body_comp = scene->assign_component<BodyComponent>(mesh_ent);

				BoxShapeDescription box_shape
				{
					.half_extent = glm::vec3(500.0f, 0.1f, 500.0f)
				};
				set_body_shape(body_comp, box_shape);
				set_body_rotation(body_comp, glm::vec3(0.0f, glm::radians(-90.0f), 0.0f));
			}

			// Add drones
			for (uint32_t i = 0; i < 3; ++i)
			{
				EntityID mesh_ent = scene->make_entity();

				TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(mesh_ent);

				set_rotation(transform_comp, glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
				set_position(transform_comp, glm::vec3(0.0f, 30.0f, i * 25.0f - 17.0f));
				set_scale(transform_comp, glm::vec3(0.1f));

				MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(mesh_ent);

				MaterialID drone_material = MaterialFactory::create(
					Renderer::get()->context(),
					{
						.uniform_emissive{ 25.0f },
						.textures =
						{
							"../../assets/drone_color.sun",
							"../../assets/drone_normal.sun",
							"../../assets/drone_roughness.sun",
							"../../assets/default_black.sun",
							"../../assets/default_white.sun",
							"../../assets/drone_emissive.sun"
						}
					}
				);

				set_mesh(mesh_comp, MeshFactory::load(Renderer::get()->context(), "../../assets/drone.sun"));
				set_material(mesh_comp, drone_material);

				BodyComponent* const body_comp = scene->assign_component<BodyComponent>(mesh_ent);

				BoxShapeDescription box_shape
				{
					.half_extent = glm::vec3(2.5f, 2.5f, 2.5f)
				};
				set_body_shape(body_comp, box_shape);
				set_body_type(body_comp, PhysicsBodyType::Dynamic);
				set_body_gravity_scale(body_comp, 1.0f);
				set_body_restitution(body_comp, 0.5f);
			}

			// Add containers
			{
				MaterialID container_material = MaterialFactory::create(
					Renderer::get()->context(),
					{
						.textures =
						{
							"../../assets/container_albedo.sun",
							"../../assets/container_normal.sun",
							"../../assets/container_roughness.sun",
							"../../assets/container_metallic.sun",
							"../../assets/default_white.sun",
							"../../assets/default_black.sun"
						}
					}
				);
				MaterialID container_bar_material = MaterialFactory::create(
					Renderer::get()->context(),
					{
						.textures =
						{
							"../../assets/container_bar_albedo.sun",
							"../../assets/container_bar_normal.sun",
							"../../assets/container_bar_roughness.sun",
							"../../assets/container_bar_metallic.sun",
							"../../assets/default_white.sun",
							"../../assets/default_black.sun"
						}
					}
				);

				const MeshID container_mesh = MeshFactory::load(Renderer::get()->context(), "../../assets/container.sun");

				{
					EntityID mesh_ent = scene->make_entity();

					TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(mesh_ent);

					set_position(transform_comp, glm::vec3(0.0f, 1.0f, 75.0f));
					set_scale(transform_comp, glm::vec3(0.1f));

					MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(mesh_ent);

					set_mesh(mesh_comp, container_mesh);
					set_material(mesh_comp, container_material);
					set_material(mesh_comp, container_bar_material, 1);
					set_material(mesh_comp, container_bar_material, 3);
				}
				{
					EntityID mesh_ent = scene->make_entity();

					TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(mesh_ent);

					set_position(transform_comp, glm::vec3(0.0f, 1.0f, -75.0f));
					set_scale(transform_comp, glm::vec3(0.1f));

					MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(mesh_ent);

					set_mesh(mesh_comp, container_mesh);
					set_material(mesh_comp, container_material);
					set_material(mesh_comp, container_bar_material, 1);
					set_material(mesh_comp, container_bar_material, 3);
				}
				{
					EntityID mesh_ent = scene->make_entity();

					TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(mesh_ent);

					set_rotation(transform_comp, glm::vec3(0.0f, glm::radians(90.0f), 0.0f));
					set_position(transform_comp, glm::vec3(-75.0f, 1.0f, 0.0f));
					set_scale(transform_comp, glm::vec3(0.1f));

					MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(mesh_ent);

					set_mesh(mesh_comp, container_mesh);
					set_material(mesh_comp, container_material);
					set_material(mesh_comp, container_bar_material, 1);
					set_material(mesh_comp, container_bar_material, 3);
				}
			}

			SimulationCore::get()->register_layer(std::move(scene));
		}
	}

	void Application::cleanup()
	{	
		if (bIsInitialized)
		{
			Renderer::get()->destroy();

			SimulationCore::get()->destroy();

			window->destroy();
		}
	}

	void Application::run()
	{
		if (bIsInitialized)
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
	}
}

