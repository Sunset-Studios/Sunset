#pragma once

#include <vector>
#include <vertex_types.h>
#include <functional>
#include <utility/maths.h>

namespace Sunset
{
	using PipelineStateID = size_t;

	enum class PipelineShaderStageType : uint16_t
	{
		Vertex = 1,
		Fragment = 2,
		Geometry = 4,
		Compute = 8
	};

	using PipelineShaderPathList = std::vector<std::pair<PipelineShaderStageType, const char*>>;

	struct PipelineShaderStage
	{
		public:
			PipelineShaderStageType stage_type{ PipelineShaderStageType::Vertex };
			class Shader* shader_module{ nullptr };

			bool operator==(const PipelineShaderStage& other) const
			{
				return stage_type == other.stage_type
					&& shader_module == other.shader_module;
			}
	};

	enum class PipelinePrimitiveTopologyType
	{
		LineList,
		LineStrip,
		PointList,
		TriangleFan,
		TriangleList,
		TriangleStrip
	};

	enum class PipelineRasterizerPolygonMode
	{
		Fill,
		Line,
		Point
	};

	enum class PipelineRasterizerCullMode
	{
		None,
		FrontFace,
		BackFace,
		FrontAndBackFace
	};

	struct PipelineRasterizerState
	{
		public:
			float line_width{ 1.0f };
			PipelineRasterizerPolygonMode polygon_draw_mode{ PipelineRasterizerPolygonMode::Fill };
			PipelineRasterizerCullMode cull_mode{ PipelineRasterizerCullMode::None };

			bool operator==(const PipelineRasterizerState& other) const
			{
				return line_width == other.line_width
					&& polygon_draw_mode == other.polygon_draw_mode
					&& cull_mode == other.cull_mode;
			}
	};

	struct PipelineVertexInputDescription
	{
		public:
			std::vector<VertexBinding> bindings;
			std::vector<VertexAttribute> attributes;

			bool operator==(const PipelineVertexInputDescription& other) const
			{
				return bindings == other.bindings
					&& attributes == other.attributes;
			}
	};

	struct PipelineStateData
	{
		public:
			std::vector<PipelineShaderStage> shader_stages;
			std::vector<Viewport> viewports;
			std::vector<Scissor> scissors;
			PipelineVertexInputDescription vertex_input_description;
			class ShaderPipelineLayout* layout{ nullptr };
			PipelinePrimitiveTopologyType primitive_topology_type{ PipelinePrimitiveTopologyType::TriangleList };
			PipelineRasterizerState rasterizer_state{{}};
			uint16_t multisample_count{ 1 };
			bool b_depth_test_enabled{ false };
			bool b_depth_write_enabled{ false };
			CompareOperation compare_op{ CompareOperation::Always };

			bool operator==(const PipelineStateData& other) const
			{
				return shader_stages == other.shader_stages
					&& viewports == other.viewports
					&& scissors == other.scissors
					&& vertex_input_description == other.vertex_input_description
					&& layout == other.layout
					&& primitive_topology_type == other.primitive_topology_type
					&& rasterizer_state == other.rasterizer_state
					&& multisample_count == other.multisample_count
					&& b_depth_test_enabled == other.b_depth_test_enabled
					&& b_depth_write_enabled == other.b_depth_write_enabled
					&& compare_op == other.compare_op;
			}
	};
}

#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4267)

template<>
struct std::hash<Sunset::PipelineStateData>
{
	std::size_t operator()(const Sunset::PipelineStateData& psd) const
	{
		std::size_t ss_seed = psd.shader_stages.size();
		for (auto& i : psd.shader_stages)
		{
			std::size_t hash = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(i.stage_type), reinterpret_cast<uintptr_t>(i.shader_module));
			ss_seed ^= hash + 0x9e3779b9 + (ss_seed << 6) + (ss_seed >> 2);
		}

		std::size_t viewport_seed = psd.viewports.size();
		for (auto& i : psd.viewports)
		{
			std::size_t hash = Sunset::Maths::cantor_pair_hash(*(int32_t*)&(i.x), *(int32_t*)&(i.y));
			hash = Sunset::Maths::cantor_pair_hash(hash, *(int32_t*)&(i.width));
			hash = Sunset::Maths::cantor_pair_hash(hash, *(int32_t*)&(i.height));
			hash = Sunset::Maths::cantor_pair_hash(hash, *(int32_t*)&(i.min_depth));
			hash = Sunset::Maths::cantor_pair_hash(hash, *(int32_t*)&(i.max_depth));
			viewport_seed ^= hash + 0x9e3779b9 + (viewport_seed << 6) + (viewport_seed >> 2);
		}

		std::size_t scissors_seed = psd.scissors.size();
		for (auto& i : psd.scissors)
		{
			std::size_t hash = Sunset::Maths::cantor_pair_hash(*(int32_t*)&(i.x), *(int32_t*)&(i.y));
			hash = Sunset::Maths::cantor_pair_hash(hash, *(int32_t*)&(i.w));
			hash = Sunset::Maths::cantor_pair_hash(hash, *(int32_t*)&(i.h));
			scissors_seed ^= hash + 0x9e3779b9 + (scissors_seed << 6) + (scissors_seed >> 2);
		}

		std::size_t vertex_input_seed = psd.vertex_input_description.attributes.size() + psd.vertex_input_description.bindings.size();
		{
			for (auto& i : psd.vertex_input_description.attributes)
			{
				std::size_t hash = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(i.index), static_cast<int32_t>(i.binding));
				hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(i.data_offset));
				hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(i.format));
				vertex_input_seed ^= hash + 0x9e3779b9 + (vertex_input_seed << 6) + (vertex_input_seed >> 2);
			}
			for (auto& i : psd.vertex_input_description.bindings)
			{
				std::size_t hash = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(i.index), static_cast<int32_t>(i.stride));
				vertex_input_seed ^= hash + 0x9e3779b9 + (vertex_input_seed << 6) + (vertex_input_seed >> 2);
			}
		}

		std::size_t rasterizer_state_seed = 0;
		{
			std::size_t hash = Sunset::Maths::cantor_pair_hash(*(int32_t*)&(psd.rasterizer_state.line_width), static_cast<int32_t>(psd.rasterizer_state.cull_mode));
			hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(psd.rasterizer_state.polygon_draw_mode));
			rasterizer_state_seed ^= hash + 0x9e3779b9 + (rasterizer_state_seed << 6) + (rasterizer_state_seed >> 2);
		}

		std::size_t pipeline_layout_seed = reinterpret_cast<uintptr_t>(psd.layout);

		std::size_t topology_seed = static_cast<int32_t>(psd.primitive_topology_type);

		std::size_t multisample_seed = static_cast<int32_t>(psd.multisample_count);

		std::size_t depth_test_seed = static_cast<int32_t>(psd.b_depth_test_enabled);

		std::size_t depth_write_seed = static_cast<int32_t>(psd.b_depth_write_enabled);

		std::size_t compare_op_seed = static_cast<int32_t>(psd.compare_op);

		std::size_t final_hash = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(ss_seed), static_cast<int32_t>(viewport_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(scissors_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(vertex_input_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(pipeline_layout_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(topology_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(rasterizer_state_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(multisample_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(depth_test_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(depth_write_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(compare_op_seed));

		return final_hash;
	}
};

#pragma warning( pop ) 