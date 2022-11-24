#include <graphics/resource/shader_pipeline_layout.h>

namespace Sunset
{
	Sunset::ShaderPipelineLayout* ShaderPipelineLayoutFactory::get_default(class GraphicsContext* const gfx_context)
	{
		static ShaderPipelineLayout* layout = new ShaderPipelineLayout;
		layout->initialize(gfx_context);
		return layout;
	}

	Sunset::ShaderPipelineLayout* ShaderPipelineLayoutFactory::create(class GraphicsContext* const gfx_context, const PushConstantPipelineData& push_constant_data /*= {}*/)
	{
		ShaderPipelineLayout* layout = new ShaderPipelineLayout;
		layout->initialize(gfx_context, push_constant_data);
		return layout;
	}
}
