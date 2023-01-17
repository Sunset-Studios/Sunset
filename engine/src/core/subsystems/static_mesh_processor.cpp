#include <core/subsystems/static_mesh_processor.h>
#include <core/ecs/components/mesh_component.h>
#include <core/ecs/components/transform_component.h>
#include <core/ecs/components/camera_control_component.h>
#include <core/layers/scene.h>
#include <core/data_globals.h>
#include <graphics/resource_state.h>
#include <graphics/pipeline_state.h>
#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/mesh.h>
#include <graphics/render_pass.h>
#include <graphics/push_constants.h>
#include <graphics/resource/buffer.h>
#include <graphics/descriptor.h>
#include <graphics/resource/image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Sunset
{
	void StaticMeshProcessor::update(class Scene* scene, double delta_time)
	{
		const uint32_t current_buffered_frame = Renderer::get()->context()->get_buffered_frame_number();

		for (EntityID entity : SceneView<MeshComponent, TransformComponent>(*scene))
		{
			const int32_t entity_index = get_entity_index(entity);

			MeshComponent* const mesh_comp = scene->get_component<MeshComponent>(entity);
			TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity);

			EntityGlobals::get()->transforms.entity_transforms[entity_index] = transform_comp->transform.local_matrix;

			if (mesh_comp->resource_state == 0)
			{
				mesh_comp->resource_state = ResourceStateBuilder::create()
					.set_vertex_buffer(mesh_vertex_buffer(mesh_comp))
					.set_index_buffer(mesh_index_buffer(mesh_comp))
					.set_instance_index(entity_index)
					.set_vertex_count(mesh_vertex_count(mesh_comp))
					.set_index_count(mesh_index_count(mesh_comp))
					.finish();
			}

			Material* const material = MaterialCache::get()->fetch(mesh_comp->material);
			assert(material != nullptr && "Cannot process mesh with a null material");

			material->descriptor_datas[static_cast<int16_t>(DescriptorSetType::Global)] = Renderer::get()->get_global_descriptor_data(current_buffered_frame);
			material->descriptor_datas[static_cast<int16_t>(DescriptorSetType::Object)] = Renderer::get()->get_object_descriptor_data(current_buffered_frame);

			DescriptorData& material_descriptor = material->descriptor_datas[static_cast<int16_t>(DescriptorSetType::Material)];

			// TODO_BEGIN: Material stuff can probably be moved out into some sort of material factory or material processor
			if (material_descriptor.descriptor_set == nullptr)
			{
				std::vector<DescriptorBuildData> texture_bindings;
				for (int i = 0; i < material->textures.size(); ++i)
				{
					const char* path = material->textures[i];
					Image* const texture = ImageFactory::load(Renderer::get()->context(), path);
					texture_bindings.push_back(
					DescriptorBuildData{
						.binding = static_cast<uint16_t>(i),
						.image = texture,
						.type = DescriptorType::Image,
						.shader_stages = PipelineShaderStageType::Fragment
					});
				}
				DescriptorHelpers::inject_descriptors(Renderer::get()->context(), material_descriptor, texture_bindings);
			}

			PushConstantPipelineData push_constants_data = PushConstantPipelineData::create(&mesh_comp->additional_data);

			if (material->pipeline_state == 0)
			{
				PipelineStateBuilder state_builder = PipelineStateBuilder::create_default(Renderer::get()->window())
					.clear_shader_stages()
					.set_shader_layout(
						ShaderPipelineLayoutFactory::create(
							Renderer::get()->context(),
							push_constants_data,
							{
								Renderer::get()->global_descriptor_layout(current_buffered_frame),
								Renderer::get()->object_descriptor_layout(current_buffered_frame),
								material_descriptor.descriptor_layout
							}
						)
					)
					.value();

				for (const std::pair<PipelineShaderStageType, const char*>& shader : material->shaders)
				{
					state_builder.set_shader_stage(shader.first, shader.second);
				}

				material->pipeline_state = state_builder.finish();

				PipelineStateCache::get()->fetch(material->pipeline_state)->build(Renderer::get()->context(), Renderer::get()->master_pass()->get_data());
			}
			// TODO_END: Material stuff can probably be moved out into some sort of material factory or material processor

			Renderer::get()
				->fresh_rendertask()
				->setup(mesh_comp->material, mesh_comp->resource_state)
				->set_push_constants(std::move(push_constants_data))
				->submit(Renderer::get()->master_pass());
		}

		EntityGlobals::get()->transforms.transform_buffer[current_buffered_frame]->copy_from(
			Renderer::get()->context(),
			EntityGlobals::get()->transforms.entity_transforms.data(),
			EntityGlobals::get()->transforms.entity_transforms.size() * sizeof(glm::mat4)
		);
	}
}
