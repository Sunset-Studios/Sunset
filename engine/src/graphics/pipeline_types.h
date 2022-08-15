#pragma once

#include <vector>

namespace Sunset
{
	enum class PipelineShaderStageType
	{
		Vertex,
		Fragment,
		Geometry,
		Compute
	};

	struct PipelineShaderStage
	{
		public:
			PipelineShaderStageType stage_type{ PipelineShaderStageType::Vertex };
			class Shader* shader_module{ nullptr };
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
	};

	struct PipelineStateData
	{
		public:
			std::vector<PipelineShaderStage> shader_stages;
			std::vector<Viewport> viewports;
			std::vector<Scissor> scissors;
			class ShaderPipelineLayout* layout{ nullptr };
			PipelinePrimitiveTopologyType primitive_topology_type{ PipelinePrimitiveTopologyType::TriangleList };
			PipelineRasterizerState rasterizer_state{{}};
			uint16_t multisample_count{ 1 };
	};
}