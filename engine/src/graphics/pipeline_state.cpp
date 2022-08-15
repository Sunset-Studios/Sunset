#include <graphics/pipeline_state.h>
#include <graphics/render_pass.h>

namespace Sunset
{
	Sunset::PipelineStateBuilder& PipelineStateBuilder::add_viewport(float x_pos, float y_pos, float width, float height, float min_depth, float max_depth)
	{
		pipeline_state->add_viewport(context, x_pos, y_pos, width, height, min_depth, max_depth);
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::add_scissor(int32_t x_pos, int32_t y_pos, int32_t width, int32_t height)
	{
		pipeline_state->add_scissor(context, x_pos, y_pos, width, height);
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::set_shader_layout(class ShaderPipelineLayout* layout)
	{
		pipeline_state->set_shader_layout(context, layout);
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::set_shader_stage(PipelineShaderStageType stage, class Shader* shader)
	{
		pipeline_state->set_shader_stage(context, stage, shader);
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::set_shader_stage(PipelineShaderStageType stage, const char* shader_path)
	{
		pipeline_state->set_shader_stage(context, stage, shader_path);
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::set_primitive_topology_type(PipelinePrimitiveTopologyType topology_type)
	{
		pipeline_state->set_primitive_topology(context, topology_type);
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::set_rasterizer_state(PipelineRasterizerPolygonMode polygon_mode, float line_width, PipelineRasterizerCullMode cull_mode)
	{
		pipeline_state->set_rasterizer_state(context, polygon_mode, line_width, cull_mode);
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::set_multisample_count(uint16_t count)
	{
		pipeline_state->set_multisample_count(context, count);
		return *this;
	}

	Sunset::PipelineState* PipelineStateBuilder::build(RenderPass* render_pass)
	{
		pipeline_state->build(context, render_pass);
		return pipeline_state;
	}
}
