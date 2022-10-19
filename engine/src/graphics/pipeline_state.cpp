#include <graphics/pipeline_state.h>
#include <graphics/render_pass.h>
#include <graphics/asset_pool.h>

namespace Sunset
{

	Sunset::PipelineStateBuilder PipelineStateBuilder::create(GraphicsContext* const gfx_context)
	{
		PipelineStateBuilder state_builder;
		state_builder.context = gfx_context;
		return state_builder;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::add_viewport(float x_pos, float y_pos, float width, float height, float min_depth, float max_depth)
	{
		state_data.viewports.emplace_back(x_pos, y_pos, width, height, min_depth, max_depth);
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::add_scissor(int32_t x_pos, int32_t y_pos, int32_t width, int32_t height)
	{
		state_data.scissors.emplace_back(x_pos, y_pos, width, height);
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::set_shader_layout(class ShaderPipelineLayout* layout)
	{
		state_data.layout = layout;
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::clear_shader_stages()
	{
		state_data.shader_stages.clear();
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::set_shader_stage(PipelineShaderStageType stage, class Shader* shader)
	{
		state_data.shader_stages.emplace_back(stage, shader);
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::set_shader_stage(PipelineShaderStageType stage, const char* shader_path)
	{
		state_data.shader_stages.emplace_back(stage, ShaderFactory::create(context, shader_path));
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::set_vertex_input_description(PipelineVertexInputDescription vertex_input_description)
	{
		state_data.vertex_input_description = vertex_input_description;
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::set_primitive_topology_type(PipelinePrimitiveTopologyType topology_type)
	{
		state_data.primitive_topology_type = topology_type;
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::set_rasterizer_state(PipelineRasterizerPolygonMode polygon_mode, float line_width, PipelineRasterizerCullMode cull_mode)
	{
		state_data.rasterizer_state.polygon_draw_mode = polygon_mode;
		state_data.rasterizer_state.line_width = line_width;
		state_data.rasterizer_state.cull_mode = cull_mode;
		return *this;
	}

	Sunset::PipelineStateBuilder& PipelineStateBuilder::set_multisample_count(uint16_t count)
	{
		state_data.multisample_count = count;
		return *this;
	}

	Sunset::PipelineStateID PipelineStateBuilder::finish()
	{
		return PipelineStateCache::get()->fetch_or_add(state_data, context);
	}

	void PipelineStateCache::remove(PipelineStateID id)
	{
		cache.erase(id);
	}

	PipelineStateID PipelineStateCache::fetch_or_add(const PipelineStateData& data, class GraphicsContext* const gfx_context)
	{
		PipelineStateID id = std::hash<PipelineStateData>{}(data);
		if (cache.find(id) == cache.end())
		{
			PipelineState* const new_pipeline_state = GlobalAssetPools<PipelineState>::get()->allocate();
			new_pipeline_state->initialize(gfx_context, data);
			cache.insert({ id, new_pipeline_state });
		}
		return id;
	}

	Sunset::PipelineState* PipelineStateCache::fetch(PipelineStateID id)
	{
		assert(cache.find(id) != cache.end());
		return cache[id];
	}

	void PipelineStateCache::initialize()
	{
	}

	void PipelineStateCache::update()
	{
	}

	void PipelineStateCache::destroy(GraphicsContext* const gfx_context)
	{
		for (const std::pair<size_t, PipelineState*>& pair : cache)
		{
			pair.second->destroy(gfx_context);
		}
		cache.clear();
	}
}
