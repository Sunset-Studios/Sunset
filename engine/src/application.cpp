#include <application.h>
#include <window/window.h>
#include <graphics/renderer.h>
#include <input/input_provider.h>
#include <core/simulation_core.h>
#include <core/layers/scene.h>
#include <graphics/resource/mesh.h>

#include <core/ecs/components/mesh_component.h>

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

		std::unique_ptr<Scene> scene = std::make_unique<Scene>();

		EntityID mesh_ent = scene->make_entity();
		MeshComponent* const mesh_comp = scene->assign_component<MeshComponent>(mesh_ent);

		set_mesh(mesh_comp,
			MeshFactory::load_obj(Renderer::get()->context(), "../../assets/monkey_smooth.obj"));
		set_shaders(mesh_comp,
			{
				{PipelineShaderStageType::Vertex, "../../shaders/basic_colored.vert.spv"},
				{PipelineShaderStageType::Fragment, "../../shaders/basic_colored.frag.spv"}
			});

		SimulationCore::get()->register_layer(std::move(scene));
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

