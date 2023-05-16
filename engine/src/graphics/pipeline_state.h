#pragma once

#include <common.h>
#include <pipeline_types.h>
#include <graphics/render_pass_types.h>
#include <graphics/resource/shader.h>
#include <graphics/resource/shader_pipeline_layout.h>
#include <graphics/resource/resource_cache.h>

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

		void initialize(class GraphicsContext* const gfx_context, const PipelineStateData& data)
		{
			state_data = data;
			pipeline_state_policy.initialize(gfx_context, &state_data);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			pipeline_state_policy.destroy(gfx_context, &state_data);
		}

		void build(class GraphicsContext* const gfx_context, class RenderPass* render_pass)
		{
			pipeline_state_policy.build(gfx_context, &state_data, render_pass);
		}

		void build_compute(class GraphicsContext* const gfx_context, void* render_pass_data)
		{
			pipeline_state_policy.build_compute(gfx_context, &state_data, render_pass_data);
		}

		void bind(class GraphicsContext* const gfx_context, void* buffer)
		{
			pipeline_state_policy.bind(gfx_context, state_data.type, buffer);
		}

		void* get_handle()
		{
			return pipeline_state_policy.get_handle();
		}

		PipelineStateData& get_state_data()
		{
			return state_data;
		}

		void clear_viewports(class GraphicsContext* const gfx_context)
		{
			state_data.viewports.clear();
		}

		void add_viewport(class GraphicsContext* const gfx_context, float x_pos, float y_pos, float width, float height, float min_depth, float max_depth)
		{
			state_data.viewports.emplace_back(x_pos, y_pos, width, height, min_depth, max_depth);
		}

		void add_viewport(class GraphicsContext* const gfx_context, const Viewport& viewport)
		{
			state_data.viewports.emplace_back(viewport);
		}

		void add_scissor(class GraphicsContext* const gfx_context, int32_t x_pos, int32_t y_pos, int32_t width, int32_t height)
		{
			state_data.scissors.emplace_back(x_pos, y_pos, width, height);
		}

		void clear_shader_stages(class GraphicsContext* const gfx_context)
		{
			state_data.shader_stages.clear();
		}

		void set_shader_layout(class GraphicsContext* const gfx_context, ShaderLayoutID layout)
		{
			state_data.layout = layout;
		}

		void set_vertex_input_description(class GraphicsContext* const gfx_context, PipelineVertexInputDescription vertex_input_description)
		{
			state_data.vertex_input_description = vertex_input_description;
		}

		void set_shader_stage(class GraphicsContext* const gfx_context, PipelineShaderStageType stage, ShaderID shader)
		{
			state_data.shader_stages.emplace_back(stage, shader);
		}

		void set_shader_stage(class GraphicsContext* const gfx_context, PipelineShaderStageType stage, const char* shader_path)
		{
			bool b_added{ false };
			const ShaderID new_shader = ShaderCache::get()->fetch_or_add(shader_path, gfx_context, b_added);
			if (b_added)
			{
				Shader* const shader = CACHE_FETCH(Shader, new_shader);
				if (!shader->is_compiled())
				{
					shader->initialize(gfx_context, shader_path);
				}
			}
			state_data.shader_stages.emplace_back(stage, new_shader);
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

		void set_attachment_blend_state(const PipelineAttachmentBlendState& blend_state)
		{
			state_data.attachment_blend_state = blend_state;
		}

		void set_multisample_count(class GraphicsContext* const gfx_context, uint16_t count)
		{
			state_data.multisample_count = count;
		}

		void set_depth_stencil_state(class GraphicsContext* const gfx_context, bool b_depth_test_enabled, bool b_depth_write_enabled, CompareOperation compare_op)
		{
			state_data.b_depth_test_enabled = b_depth_test_enabled;
			state_data.b_depth_write_enabled = b_depth_write_enabled;
			state_data.compare_op = compare_op;
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

		void build(class GraphicsContext* const gfx_context, PipelineStateData* state_data, class RenderPass* render_pass)
		{ }

		void build_compute(class GraphicsContext* const gfx_context, struct PipelineStateData* state_data, void* render_pass_data)
		{ }

		void bind(class GraphicsContext* const gfx_context, PipelineStateType type, void* buffer)
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

	class PipelineGraphicsStateBuilder
	{
		public:
			static PipelineGraphicsStateBuilder create();
			static PipelineGraphicsStateBuilder create(const PipelineStateData& data);
			static PipelineGraphicsStateBuilder create_default(const glm::ivec2& resolution);

			PipelineGraphicsStateBuilder& clear_viewports();
			PipelineGraphicsStateBuilder& add_viewport(const Viewport& viewport);
			PipelineGraphicsStateBuilder& add_viewport(float x_pos, float y_pos, float width, float height, float min_depth, float max_depth);
			PipelineGraphicsStateBuilder& add_scissor(int32_t x_pos, int32_t y_pos, int32_t width, int32_t height);
			PipelineGraphicsStateBuilder& set_shader_layout(ShaderLayoutID layout);
			PipelineGraphicsStateBuilder& clear_shader_stages();
			PipelineGraphicsStateBuilder& set_shader_stage(PipelineShaderStageType stage, ShaderID shader);
			PipelineGraphicsStateBuilder& set_shader_stage(PipelineShaderStageType stage, const char* shader_path);
			PipelineGraphicsStateBuilder& set_vertex_input_description(PipelineVertexInputDescription vertex_input_description);
			PipelineGraphicsStateBuilder& set_primitive_topology_type(PipelinePrimitiveTopologyType topology_type);
			PipelineGraphicsStateBuilder& set_rasterizer_state(PipelineRasterizerPolygonMode polygon_mode, float line_width, PipelineRasterizerCullMode cull_mode);
			PipelineGraphicsStateBuilder& set_rasterizer_state(const PipelineRasterizerState& rasterizer_state);
			PipelineGraphicsStateBuilder& set_attachment_blend_state(const PipelineAttachmentBlendState& blend_state);
			PipelineGraphicsStateBuilder& set_multisample_count(uint16_t count);
			PipelineGraphicsStateBuilder& set_depth_stencil_state(bool b_depth_test_enabled, bool b_depth_write_enabled, CompareOperation compare_op);
			PipelineGraphicsStateBuilder& set_pass(RenderPassID pass);
			PipelineGraphicsStateBuilder& set_push_constants(const PushConstantPipelineData& push_constant_data);
			PipelineGraphicsStateBuilder& derive_shader_layout(std::vector<DescriptorLayoutID>& out_descriptor_layouts);

			PipelineStateID finish();

			PipelineGraphicsStateBuilder value() const
			{
				return *this;
			}

			PipelineStateID get_state() const
			{
				return pipeline_state;
			}

		protected:
			PipelineStateData state_data;
			PipelineStateID pipeline_state;
			class GraphicsContext* context{ nullptr };
	};

	class PipelineComputeStateBuilder
	{
	public:
		static PipelineComputeStateBuilder create();
		static PipelineComputeStateBuilder create(const PipelineStateData& data);

		PipelineComputeStateBuilder& set_shader_layout(ShaderLayoutID layout);
		PipelineComputeStateBuilder& clear_shader_stages();
		PipelineComputeStateBuilder& set_shader_stage(ShaderID shader);
		PipelineComputeStateBuilder& set_shader_stage(const char* shader_path);
		PipelineComputeStateBuilder& set_pass(RenderPassID pass);
		PipelineComputeStateBuilder& set_push_constants(const PushConstantPipelineData& push_constant_data);
		PipelineComputeStateBuilder& derive_shader_layout(std::vector<DescriptorLayoutID>& out_descriptor_layouts);

		PipelineStateID finish();

		PipelineComputeStateBuilder value() const
		{
			return *this;
		}

		PipelineStateID get_state() const
		{
			return pipeline_state;
		}

	protected:
		PipelineStateData state_data;
		PipelineStateID pipeline_state;
		class GraphicsContext* context{ nullptr };
	};

	DEFINE_RESOURCE_CACHE(PipelineStateCache, PipelineStateID, PipelineState);
}
