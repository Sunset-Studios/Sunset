#include <application.h>
#include <window/window.h>
#include <graphics/renderer.h>
#include <input/input_provider.h>
#include <core/simulation_core.h>
#include <core/layers/scene.h>
#include <core/layers/editor_gui.h>
#include <graphics/resource/mesh.h>

#include <core/ecs/components/mesh_component.h>
#include <core/ecs/components/transform_component.h>

#include <SDL.h>
#include <SDL_vulkan.h>

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

		//{ // Test Scene 1
		//	std::unique_ptr<Scene> scene = std::make_unique<Scene>();

		//	for (int i = -10; i < 10; ++i)
		//	{
		//		for (int j = -10; j < 10; ++j)
		//		{
		//			EntityID mesh_ent = scene->make_entity();
		//		
		//			TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(mesh_ent);

		//			set_position(transform_comp, glm::vec3(i * 2.0f + 1.0f, j * 2.0f, -10.0f));
		//			set_scale(transform_comp, glm::vec3(1.0f));

		//			MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(mesh_ent);

		//			set_mesh(mesh_comp, MeshFactory::load_obj(Renderer::get()->context(), "../../assets/monkey_smooth.sun"));
		//			set_shaders(mesh_comp,
		//			{
		//				{PipelineShaderStageType::Vertex, "../../shaders/default_mesh.vert.spv"},
		//				{PipelineShaderStageType::Fragment, "../../shaders/default_lit.frag.spv"}
		//			});
		//		}
		//	}

		//	SimulationCore::get()->register_layer(std::move(scene));
		//}

		{ // Test Scene 2
			std::unique_ptr<Scene> scene = std::make_unique<Scene>();

			EntityID mesh_ent = scene->make_entity();

			TransformComponent* const transform_comp = scene->assign_component<TransformComponent>(mesh_ent);

			set_position(transform_comp, glm::vec3(5.0f, -10.0f, 0.0f));
			set_scale(transform_comp, glm::vec3(1.0f));

			MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(mesh_ent);

			Material mesh_material
			{
				.shaders =
				{
					{PipelineShaderStageType::Vertex, "../../shaders/default_mesh.vert.spv"},
					{PipelineShaderStageType::Fragment, "../../shaders/default_lit.frag.spv"}
				},
				.textures =
				{
					"../../assets/lost_empire-RGBA.sun"
				}
			};

			set_mesh(mesh_comp, MeshFactory::load(Renderer::get()->context(), "../../assets/lost_empire.sun"));
			set_material(mesh_comp, mesh_material);

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

				InputProvider::get()->update();

				SimulationCore::get()->update();

				Renderer::get()->draw();
			}
		}
	}
}

