#include <graphics/debug_draw_helpers.h>
#include <graphics/renderer.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/mesh.h>
#include <graphics/resource_state.h>
#include <graphics/pipeline_state.h>
#include <graphics/resource/buffer.h>
#include <glm/gtx/rotate_vector.hpp>

namespace Sunset
{
	void DebugDrawExecutor::operator()(
		GraphicsContext* const gfx_context,
		void* command_buffer,
		ResourceStateID resource_state,
		PipelineStateID pipeline_state,
		int32_t buffered_frame_number,
		uint32_t instance_count,
		const PushConstantPipelineData& push_constants)
	{
		CACHE_FETCH(ResourceState, resource_state)->bind(gfx_context, command_buffer);

		if (push_constants.data != nullptr)
		{
			gfx_context->push_constants(command_buffer, pipeline_state, push_constants);
		}

		gfx_context->draw_indexed(
			command_buffer,
			static_cast<uint32_t>(CACHE_FETCH(ResourceState, resource_state)->state_data.index_count),
			instance_count,
			0);
	}

	void DebugDrawState::initialize()
	{
		ZoneScopedN("DebugDrawState::initialize");

		std::string buffer_name = "debug_primitive_datas0";
		for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			if (primitive_data_buffers[i] == 0)
			{
				buffer_name[buffer_name.size() - 1] = '0' + i;
				primitive_data_buffers[i] = BufferFactory::create(
					Renderer::get()->context(),
					{
						.name = buffer_name.c_str(),
						.buffer_size = sizeof(DebugPrimitiveData) * MIN_DEBUG_DRAW_PRIMITIVES,
						.type = BufferType::StorageBuffer
					}
				);
			}
		}
	}

	void debug_draw_line(GraphicsContext* const gfx_context, glm::vec3 line_start, glm::vec3 line_end, glm::vec3 line_color, int32_t buffered_frame_number)
	{
		const glm::vec3 line = line_end - line_start;
		const float magnitude = glm::length(line);

		const glm::mat4 translation = glm::translate(glm::mat4(1.0f), line_start);
		const glm::mat4 rotation = glm::orientation(glm::normalize(line), WORLD_RIGHT);
		const glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(magnitude, 0.0f, 0.0f));

		DebugPrimitiveData* const prim_data = DebugDrawState::get()->requested_primitive_datas[buffered_frame_number].get_new();
		prim_data->transform = translation * rotation * scale;
		prim_data->color = glm::vec4(line_color, 1.0f);
	}

	void submit_requested_debug_draws_auto_shader(GraphicsContext* const gfx_context, void* command_buffer, RenderPassID render_pass, int32_t buffered_frame_number)
	{
		static PipelineStateID pipeline_state;
		if (pipeline_state == 0)
		{
			PipelineGraphicsStateBuilder state_builder = PipelineGraphicsStateBuilder::create_default(gfx_context->get_surface_resolution())
				.clear_shader_stages()
				.set_pass(render_pass)
				.value();

			state_builder.set_rasterizer_state(PipelineRasterizerPolygonMode::Line, 1.0f, PipelineRasterizerCullMode::None);
			state_builder.set_attachment_blend_state(PipelineAttachmentBlendState
				{
					.b_blend_enabled = true,
					.source_color_blend = BlendFactor::One,
					.destination_color_blend = BlendFactor::Zero,
					.color_blend_op = BlendOp::Add
				});
			state_builder.set_depth_stencil_state(true, false, CompareOperation::LessOrEqual);
			state_builder.set_shader_stage(PipelineShaderStageType::Vertex, "../../shaders/debug/debug_primitive.vert.sun");
			state_builder.set_shader_stage(PipelineShaderStageType::Fragment, "../../shaders/debug/debug_primitive_deferred.frag.sun");

			{
				std::vector<DescriptorLayoutID> descriptor_layouts;
				state_builder.derive_shader_layout(descriptor_layouts);
			}

			pipeline_state = state_builder.finish();
		}

		submit_requested_debug_draws(gfx_context, command_buffer, pipeline_state, buffered_frame_number);
	}

	void submit_requested_debug_draws(GraphicsContext* const gfx_context, void* command_buffer, PipelineStateID pipeline_state, int32_t buffered_frame_number)
	{
		const uint32_t primitive_data_count = DebugDrawState::get()->requested_primitive_datas[buffered_frame_number].size();

		Buffer* const debug_draws_buffer = CACHE_FETCH(Buffer, DebugDrawState::get()->primitive_data_buffers[buffered_frame_number]);
		debug_draws_buffer->copy_from(
			gfx_context,
			DebugDrawState::get()->requested_primitive_datas[buffered_frame_number].data(),
			DebugDrawState::get()->requested_primitive_datas[buffered_frame_number].size() * sizeof(DebugPrimitiveData)
		);

		static MeshID line_mesh_id = MeshFactory::create_line(gfx_context);
		Mesh* const line_mesh = CACHE_FETCH(Mesh, line_mesh_id);

		static ResourceStateID line_resource_state = ResourceStateBuilder::create()
			.set_vertex_buffer(line_mesh->vertex_buffer)
			.set_index_buffer(line_mesh->sections[0].index_buffer)
			.set_vertex_count(line_mesh->vertices.size())
			.set_index_count(line_mesh->sections[0].indices.size())
			.finish();

		CACHE_FETCH(PipelineState, pipeline_state)->bind(gfx_context, command_buffer);

		{
			static DebugDrawExecutor draw_executor;
			draw_executor(
				gfx_context,
				command_buffer,
				line_resource_state,
				pipeline_state,
				buffered_frame_number,
				primitive_data_count
			);
		}

		DebugDrawState::get()->requested_primitive_datas[buffered_frame_number].reset();
	}
}
