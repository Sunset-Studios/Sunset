#include <graphics/resource/shader_pipeline_layout.h>
#include <utility/maths.h>

namespace Sunset
{
	Sunset::ShaderLayoutID ShaderPipelineLayoutFactory::get_default(class GraphicsContext* const gfx_context)
	{
		static ShaderLayoutID layout;
		if (layout == 0)
		{
			bool b_added{ false };
			ShaderLayoutID layout = ShaderPipelineLayoutCache::get()->fetch_or_add("default", gfx_context, b_added);
			if (b_added)
			{
				CACHE_FETCH(ShaderPipelineLayout, layout)->initialize(gfx_context);
			}
		}
		return layout;
	}

	Sunset::ShaderLayoutID ShaderPipelineLayoutFactory::create(class GraphicsContext* const gfx_context, const std::vector<PushConstantPipelineData>& push_constant_data, const std::vector<DescriptorLayoutID> descriptor_layouts)
	{
		size_t hash = 0;
		for (const PushConstantPipelineData& pc_data : push_constant_data)
		{
			hash = Maths::cantor_pair_hash(hash, std::hash<PushConstantPipelineData>{}(pc_data));
		}
		for (DescriptorLayoutID layout : descriptor_layouts)
		{
			hash = Maths::cantor_pair_hash(hash, layout);
		}

		bool b_added{ false };
		ShaderLayoutID layout = ShaderPipelineLayoutCache::get()->fetch_or_add(hash, gfx_context, b_added);
		if (b_added)
		{
			CACHE_FETCH(ShaderPipelineLayout, layout)->initialize(gfx_context, push_constant_data, descriptor_layouts);
		}
		return layout;
	}
}
 