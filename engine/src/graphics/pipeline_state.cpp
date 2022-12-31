#include <graphics/pipeline_state.h>
#include <graphics/render_pass.h>
#include <graphics/asset_pool.h>
#include <graphics/renderer.h>
#include <window/window.h>
#include <graphics/resource/mesh.h>

namespace Sunset
{

	Sunset::PipelineStateBuilder PipelineStateBuilder::create()
	{
		PipelineStateBuilder state_builder;
		state_builder.context = Renderer::get()->context();
		return state_builder;
	}

	Sunset::PipelineStateBuilder PipelineStateBuilder::create(const PipelineStateData& data)
	{
		PipelineStateBuilder state_builder;
		state_builder.context = Renderer::get()->context();;
		state_builder.state_data = data;
		return state_builder;
	}

	Sunset::PipelineStateBuilder PipelineStateBuilder::create_default(Window* window)
	{
		return create()
			.add_viewport(0.0f, 0.0f, static_cast<float>(window->get_extent().x), static_cast<float>(window->get_extent().y), 0.0f, 1.0f)
			.add_scissor(0, 0, window->get_extent().x, window->get_extent().y)
			.set_shader_stage(PipelineShaderStageType::Vertex, "../../shaders/default_mesh.vert.spv")
			.set_shader_stage(PipelineShaderStageType::Fragment, "../../shaders/default_lit.frag.spv")
			.set_vertex_input_description(Vertex::get_description())
			.set_primitive_topology_type(PipelinePrimitiveTopologyType::TriangleList)
			.set_rasterizer_state(PipelineRasterizerPolygonMode::Fill, 1.0f, PipelineRasterizerCullMode::None)
			.set_multisample_count(1)
			.set_depth_stencil_state(true, true, CompareOperation::LessOrEqual)
			.value();
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
		// TODO: Defer creation of shaders until builder finish is called, and think about creating a separate cache for shaders
		// so we don't potentially create duplicates
		const ShaderID new_shader = ShaderCache::get()->fetch_or_add(shader_path, context);
		state_data.shader_stages.emplace_back(stage, ShaderCache::get()->fetch(new_shader));
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


	Sunset::PipelineStateBuilder& PipelineStateBuilder::set_depth_stencil_state(bool b_depth_test_enabled, bool b_depth_write_enabled, CompareOperation compare_op)
	{
		state_data.b_depth_test_enabled = b_depth_test_enabled;
		state_data.b_depth_write_enabled = b_depth_write_enabled;
		state_data.compare_op = compare_op;
		return *this;
	}

	Sunset::PipelineStateID PipelineStateBuilder::finish()
	{
		if (state_data.layout == nullptr)
		{
			state_data.layout = ShaderPipelineLayoutFactory::get_default(Renderer::get()->context());
		}
		return PipelineStateCache::get()->fetch_or_add(state_data, context);
	}

	void PipelineStateCache::initialize()
	{
	}

	void PipelineStateCache::update()
	{
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

	void PipelineStateCache::remove(PipelineStateID id)
	{
		cache.erase(id);
	}

	Sunset::PipelineState* PipelineStateCache::fetch(PipelineStateID id)
	{
		assert(cache.find(id) != cache.end());
		return cache[id];
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
