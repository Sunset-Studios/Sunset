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

#include <core/ecs/components/mesh_component.h>
#include <core/ecs/components/transform_component.h>
#include <core/ecs/components/light_component.h>

#include <SDL.h>
#include <SDL_vulkan.h>

#include <Tracy.hpp>

namespace Sunset
{
	void Application::init()
	{
		bIsInitialized = true;

		window = WindowFactory::create(ENGINE_NAME, glm::ivec2(0), glm::ivec2(1280, 720));

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

			// Add light 1
			{
				EntityID light_entity = scene->make_entity();

				TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(light_entity);

				set_position(transform_comp, glm::vec3(-10.0f, 10.0f, 0.0f));

				LightComponent* const light_comp = scene->assign_component<LightComponent>(light_entity);

				set_light_color(light_comp, glm::vec3(1.0f, 1.0f, 1.0f));
				set_light_type(light_comp, LightType::Directional);
				set_light_intensity(light_comp, 2.0f);
				set_light_entity_index(light_comp, get_entity_index(light_entity));
			}

			// Add light 2
			{
				EntityID light_entity = scene->make_entity();

				TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(light_entity);

				set_position(transform_comp, glm::vec3(25.0f, 10.0f, -50.0f));
				set_rotation(transform_comp, glm::vec3(0.0f, 45.0f, 0.0f));

				LightComponent* const light_comp = scene->assign_component<LightComponent>(light_entity);

				set_light_color(light_comp, glm::vec3(1.0f, 0.2f, 0.2f));
				set_light_type(light_comp, LightType::Point);
				set_light_radius(light_comp, 15.0f);
				set_light_intensity(light_comp, 1000.0f);
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
							"../../assets/hardwood_albedo.sun",	 // albedo
							"../../assets/hardwood_normal.sun",  // normal
							"../../assets/default_black.sun"     // roughness
						}
					}
				);
				material_set_texture_tiling(Renderer::get()->context(), mesh_material, 0, 5.0f);
				material_set_texture_tiling(Renderer::get()->context(), mesh_material, 1, 5.0f);

				set_mesh(mesh_comp, MeshFactory::create_quad(Renderer::get()->context()));
				set_material(mesh_comp, mesh_material);
			}

			// Add test mesh
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
							"../../assets/default_white.sun",	 // albedo
						}
					}
				);

				set_mesh(mesh_comp, MeshFactory::create_sphere(Renderer::get()->context(), glm::ivec2(16.0f, 16.0f), 1.0f));
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

					InputProvider::get()->update();

					SimulationCore::get()->update();
				}

				FrameMark;
			}
		}
	}
}

