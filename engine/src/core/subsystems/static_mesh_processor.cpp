#include <core/subsystems/static_mesh_processor.h>
#include <core/ecs/components/mesh_component.h>
#include <core/ecs/components/transform_component.h>
#include <core/layers/scene.h>
#include <graphics/resource_state.h>
#include <graphics/pipeline_state.h>
#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/mesh.h>
#include <graphics/render_pass.h>
#include <graphics/push_constants.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Sunset
{
	void StaticMeshProcessor::update(class Scene* scene, double delta_time)
	{
		for (EntityID entity : SceneView<MeshComponent, TransformComponent>(*scene))
		{
			MeshComponent* const mesh_comp = scene->get_component<MeshComponent>(entity);
			TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity);

			// Transform Calcs
			glm::vec3 cam_pos = { 0.0f, 0.0f, glm::sin(SECONDS_TIME) * 0.15f - 2.0f};
			glm::mat4 view = glm::translate(glm::mat4(1.0f), cam_pos);
			glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1280.0f / 720.0f, 0.1f, 200.0f);
			projection[1][1] *= -1;

			mesh_comp->uniform_data.transform_matrix = projection * view * transform_comp->transform.local_matrix;;
			// Transform Calcs

			if (mesh_comp->resource_state == 0)
			{
				mesh_comp->resource_state = ResourceStateBuilder::create()
					.set_vertex_buffer(mesh_comp->mesh->vertex_buffer)
					.finish();
			}

			PushConstantPipelineData push_constants_data = PushConstantPipelineData::create(&mesh_comp->uniform_data);

			if (mesh_comp->pipeline_state == 0)
			{
				PipelineStateBuilder state_builder = PipelineStateBuilder::create_default(Renderer::get()->window())
					.clear_shader_stages()
					.set_shader_layout(ShaderPipelineLayoutFactory::create(Renderer::get()->context(), push_constants_data))
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
				->set_push_constants(std::move(push_constants_data))
				->submit(Renderer::get()->master_pass());
		}
	}
}
