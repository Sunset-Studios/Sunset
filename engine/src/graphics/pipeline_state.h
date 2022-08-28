#pragma once

#include <common.h>
#include <pipeline_types.h>
#include <graphics/resource/shader.h>
#include <graphics/resource/shader_pipeline_layout.h>

namespace Sunset
{
	template<class Policy>
	class GenericPipelineState
	{
	public:
		GenericPipelineState() = default;

		void initialize(class GraphicsContext* const gfx_context)
		{
			pipeline_state_policy.initialize(gfx_context, &state_data);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			pipeline_state_policy.destroy(gfx_context, &state_data);
			state_data.layout->destroy(gfx_context);
		}

		void build(class GraphicsContext* const gfx_context, void* render_pass_data)
		{
			pipeline_state_policy.build(gfx_context, &state_data, render_pass_data);
		}

		void bind(class GraphicsContext* const gfx_context, void* buffer)
		{
			pipeline_state_policy.bind(gfx_context, buffer);
		}

		void* get_handle()
		{
			return pipeline_state_policy.get_handle();
		}

		void add_viewport(class GraphicsContext* const gfx_context, float x_pos, float y_pos, float width, float height, float min_depth, float max_depth)
		{
			state_data.viewports.emplace_back(x_pos, y_pos, width, height, min_depth, max_depth);
		}

		void add_scissor(class GraphicsContext* const gfx_context, int32_t x_pos, int32_t y_pos, int32_t width, int32_t height)
		{
			state_data.scissors.emplace_back(x_pos, y_pos, width, height);
		}

		void clear_shader_stages(class GraphicsContext* const gfx_context)
		{
			state_data.shader_stages.clear();
		}

		void set_shader_layout(class GraphicsContext* const gfx_context, class ShaderPipelineLayout* layout)
		{
			state_data.layout = layout;
		}

		void set_shader_stage(class GraphicsContext* const gfx_context, PipelineShaderStageType stage, class Shader* shader)
		{
			state_data.shader_stages.emplace_back(stage, shader);
		}

		void set_shader_stage(class GraphicsContext* const gfx_context, PipelineShaderStageType stage, const char* shader_path)
		{
			state_data.shader_stages.emplace_back(stage, ShaderFactory::create(gfx_context, shader_path));
		}

		void set_primitive_topology(class GraphicsContext* const gfx_context, PipelinePrimitiveTopologyType topology_type)
		{
			state_data.primitive_topology_type = topology_type;
		}

		void set_rasterizer_state(class GraphicsContext* const gfx_context, PipelineRasterizerPolygonMode polygon_mode, float line_width, PipelineRasterizerCullMode cull_mode)
		{
			state_data.rasterizer_state.polygon_draw_mode = polygon_mode;
			state_data.rasterizer_state.line_width = line_width;
			state_data.rasterizer_state.cull_mode = cull_mode;
		}

		void set_multisample_count(class GraphicsContext* const gfx_context, uint16_t count)
		{
			state_data.multisample_count = count;
		}

	private:
		Policy pipeline_state_policy;
		PipelineStateData state_data;
	};

	class NoopPipelineState
	{
	public:
		NoopPipelineState() = default;

		void initialize(class GraphicsContext* const gfx_context, PipelineStateData* state_data)
		{ }

		void destroy(class GraphicsContext* const gfx_context, PipelineStateData* state_data)
		{ }

		void build(class GraphicsContext* const gfx_context, PipelineStateData* state_data, void* render_pass_data)
		{ }

		void bind(class GraphicsContext* const gfx_context, void* buffer)
		{ }

		void* get_handle()
		{
			return nullptr;
		}
	};

#if USE_VULKAN_GRAPHICS
	class PipelineState : public GenericPipelineState<VulkanPipelineState>
	{ };
#else
	class PipelineState : public GenericPipelineState<NoopPipelineState>
	{ };
#endif

	class PipelineStateBuilder
	{
		public:
			static PipelineStateBuilder create(class GraphicsContext* const gfx_context)
			{
				PipelineStateBuilder state_builder;
				state_builder.context = gfx_context;
				state_builder.pipeline_state = new PipelineState;
				state_builder.pipeline_state->initialize(gfx_context);
				return state_builder;
			}

			PipelineStateBuilder& add_viewport(float x_pos, float y_pos, float width, float height, float min_depth, float max_depth);
			PipelineStateBuilder& add_scissor(int32_t x_pos, int32_t y_pos, int32_t width, int32_t height);
			PipelineStateBuilder& set_shader_layout(class ShaderPipelineLayout* layout);
			PipelineStateBuilder& clear_shader_stages();
			PipelineStateBuilder& set_shader_stage(PipelineShaderStageType stage, class Shader* shader);
			PipelineStateBuilder& set_shader_stage(PipelineShaderStageType stage, const char* shader_path);
			PipelineStateBuilder& set_primitive_topology_type(PipelinePrimitiveTopologyType topology_type);
			PipelineStateBuilder& set_rasterizer_state(PipelineRasterizerPolygonMode polygon_mode, float line_width, PipelineRasterizerCullMode cull_mode);
			PipelineStateBuilder& set_multisample_count(uint16_t count);

			PipelineStateBuilder value() const
			{
				return *this;
			}

			PipelineState* get_state() const
			{
				return pipeline_state;
			}

			PipelineState* build(void* render_pass_data);

		protected:
			class PipelineState* pipeline_state{ nullptr };
			class GraphicsContext* context{ nullptr };
	};

	class PipelineStateCache
	{
		public:
			PipelineStateCache() = default;

			void add(class PipelineState* pipeline_state);
			void remove(class PipelineState* pipeline_state);
			PipelineState* get(uint32_t index);
			void destroy(class GraphicsContext* const gfx_context);
			
			size_t size() const
			{
				return cache.size();
			}

		protected:
			std::vector<PipelineState*> cache;
	};
}
