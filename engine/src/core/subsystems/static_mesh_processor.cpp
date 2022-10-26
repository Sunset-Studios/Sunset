#include <core/subsystems/static_mesh_processor.h>
#include <core/ecs/components/mesh_component.h>
#include <core/layers/scene.h>
#include <graphics/resource_state.h>
#include <graphics/pipeline_state.h>
#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/mesh.h>
#include <graphics/render_pass.h>

namespace Sunset
{
	void StaticMeshProcessor::update(class Scene* scene, double delta_time)
	{
		for (EntityID entity : SceneView<MeshComponent>(*scene))
		{
			MeshComponent* const mesh_comp = scene->get_component<MeshComponent>(entity);

			// Some stuff in code for switching the pipeline state on a mesh based on current input
			//if (InputProvider::get()->get_action(InputKey::K_Space))
			//{
			//	current_pso_index = (current_pso_index + 1) % PipelineStateCache::get()->size();
			//}

			if (mesh_comp->resource_state == 0)
			{
				mesh_comp->resource_state = ResourceStateBuilder::create()
					.set_vertex_buffer(mesh_comp->mesh->vertex_buffer)
					.finish();
			}

			if (mesh_comp->pipeline_state == 0)
			{
				PipelineStateBuilder state_builder = PipelineStateBuilder::create_default(Renderer::get()->window())
					.clear_shader_stages()
					.value();

				for (const std::pair<PipelineShaderStageType, const char*>& shader : mesh_comp->shaders)
				{
					state_builder.set_shader_stage(shader.first, shader.second);
				}

				mesh_comp->pipeline_state = state_builder.finish();

				PipelineStateCache::get()->fetch(mesh_comp->pipeline_state)->build(Renderer::get()->context(), Renderer::get()->master_pass()->get_data());
			}

			Renderer::get()
				->fresh_rendertask()
				->setup(mesh_comp->pipeline_state, mesh_comp->resource_state, DrawCall{ mesh_vertex_count(mesh_comp) })
				->submit(Renderer::get()->master_pass());
		}
	}
}
