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

			// Add light 1
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

			// Add light 2
			{
				EntityID light_entity = scene->make_entity();

				TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(light_entity);

				set_position(transform_comp, glm::vec3(25.0f, 10.0f, -50.0f));

				LightComponent* const light_comp = scene->assign_component<LightComponent>(light_entity);

				set_light_color(light_comp, glm::vec3(1.0f, 0.2f, 0.2f));
				set_light_type(light_comp, LightType::Point);
				set_light_radius(light_comp, 15.0f);
				set_light_intensity(light_comp, 10000.0f);
				set_light_entity_index(light_comp, get_entity_index(light_entity));
			}

			// Add light 3
			{
				EntityID light_entity = scene->make_entity();

				TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(light_entity);

				set_position(transform_comp, glm::vec3(-20.0f, 8.5f, 0.0f));

				LightComponent* const light_comp = scene->assign_component<LightComponent>(light_entity);

				set_light_color(light_comp, glm::vec3(1.0f, 1.0f, 1.0f));
				set_light_type(light_comp, LightType::Point);
				set_light_radius(light_comp, 35.0f);
				set_light_intensity(light_comp, 2000.0f);
				set_light_entity_index(light_comp, get_entity_index(light_entity));
			}

			// Add ground plane
			{
				EntityID mesh_ent = scene->make_entity();

				TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(mesh_ent);

				set_rotation(transform_comp, glm::vec3(glm::radians(90.0f), glm::radians(0.0f), glm::radians(0.0f)));
				set_scale(transform_comp, glm::vec3(100.0f, 100.0f, 100.0f));

				MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(mesh_ent);

				MaterialID mesh_material = MaterialFactory::create(
					Renderer::get()->context(),
					{
						.textures =
						{
							"../../assets/tile_albedo.sun",
							"../../assets/tile_normal.sun",
							"../../assets/tile_roughness.sun",
							"../../assets/tile_metallic.sun",
							"../../assets/tile_ao.sun"
						}
					}
				);
				material_set_texture_tiling(Renderer::get()->context(), mesh_material, 0, 2.0f);
				material_set_texture_tiling(Renderer::get()->context(), mesh_material, 1, 2.0f);
				material_set_texture_tiling(Renderer::get()->context(), mesh_material, 2, 2.0f);
				material_set_texture_tiling(Renderer::get()->context(), mesh_material, 3, 2.0f);
				material_set_texture_tiling(Renderer::get()->context(), mesh_material, 4, 2.0f);

				set_mesh(mesh_comp, MeshFactory::create_quad(Renderer::get()->context()));
				set_material(mesh_comp, mesh_material);
			}

			// Add test mesh 1

			{
				EntityID mesh_ent = scene->make_entity();

				TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(mesh_ent);

				set_position(transform_comp, glm::vec3(0.0f, 8.5f, 0.0f));
				set_scale(transform_comp, glm::vec3(2.0f));

				MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(mesh_ent);

				MaterialID mesh_material = MaterialFactory::create(
					Renderer::get()->context(),
					{
						.textures =
						{
							"../../assets/metal-albedo.sun",
							"../../assets/metal-normal.sun",
							"../../assets/metal-roughness.sun",
							"../../assets/metal-metallic.sun",
							"../../assets/metal-ao.sun"
						}
					}
				);

				set_mesh(mesh_comp, MeshFactory::create_sphere(Renderer::get()->context(), glm::ivec2(32.0f, 32.0f), 1.0f));
				set_material(mesh_comp, mesh_material);
			}

			// Add test mesh 2
			for (uint32_t i = 0; i < 20; ++i)
			{
				EntityID mesh_ent = scene->make_entity();

				TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(mesh_ent);

				set_position(transform_comp, glm::vec3((i % 5) * 4.2f, 2.5f, (i / 5) * 4.2f));
				set_scale(transform_comp, glm::vec3(4.0f));

				MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(mesh_ent);

				MaterialID mesh_material = MaterialFactory::create(
					Renderer::get()->context(),
					{
						.color = glm::vec3(0.2f, 0.4, 0.8f),
						.uniform_roughness = 0.05f,
						.uniform_metallic = 0.0f,
						.uniform_clearcoat = 0.0f,
						.uniform_clearcoat_roughness = 0.0f,
					}
				);

				set_mesh(mesh_comp, MeshFactory::create_cube(Renderer::get()->context()));
				set_material(mesh_comp, mesh_material);
			}

			SimulationCore::get()->register_layer(std::move(scene));
		}
	}

	void Application::cleanup()
	{	
		if (bIsInitialized)
		{
			SimulationCore::get()->destroy();

			Renderer::get()->destroy();

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

