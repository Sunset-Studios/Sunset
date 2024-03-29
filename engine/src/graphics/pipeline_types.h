#pragma once

#include <graphics/viewport.h>

#include <vector>
#include <vertex_types.h>
#include <functional>
#include <utility/maths.h>
#include <pipeline_types.h>

namespace Sunset
{
	using PipelineStateID = size_t;

	enum class PipelineStateType : uint16_t
	{
		None = 0,
		Graphics = 1,
		Compute = 2
	};

	enum class PipelineShaderStageType : uint16_t
	{
		Vertex = 0x0001,
		Fragment = 0x0002,
		Geometry = 0x0004,
		Compute = 0x0008,
		All = 0xFFFF
	};

	inline PipelineShaderStageType operator|(PipelineShaderStageType a, PipelineShaderStageType b)
	{
		return static_cast<PipelineShaderStageType>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
	}

	inline PipelineShaderStageType operator&(PipelineShaderStageType a, PipelineShaderStageType b)
	{
		return static_cast<PipelineShaderStageType>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
	}

	inline PipelineShaderStageType& operator|=(PipelineShaderStageType& lhs, PipelineShaderStageType rhs)
	{
		return lhs = lhs | rhs;
	}

	inline PipelineShaderStageType& operator&=(PipelineShaderStageType& lhs, PipelineShaderStageType rhs)
	{
		return lhs = lhs & rhs;
	}

	enum class PipelineStageType : uint32_t
	{
		None = 0x00000000,
		TopOfPipe = 0x00000001,
		DrawIndirect = 0x00000002,
		VertexInput = 0x00000004,
		VertexShader = 0x00000008,
		TessellationControl = 0x00000010,
		TessellationEvaluation = 0x00000020,
		GeometryShader = 0x00000040,
		FragmentShader = 0x00000080,
		EarlyFragmentTest = 0x00000100,
		LateFragmentTest = 0x00000200,
		ColorAttachmentOutput = 0x00000400,
		ComputeShader = 0x00000800,
		Transfer = 0x00001000,
		BottomOfPipe = 0x00002000,
		Host = 0x00004000,
		AllGraphics = 0x00008000,
		AllCommands = 0x00010000
	};

	inline PipelineStageType operator|(PipelineStageType lhs, PipelineStageType rhs)
	{
		return static_cast<PipelineStageType>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
	}

	inline PipelineStageType operator&(PipelineStageType lhs, PipelineStageType rhs)
	{
		return static_cast<PipelineStageType>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
	}

	inline PipelineStageType& operator|=(PipelineStageType& lhs, PipelineStageType rhs)
	{
		return lhs = lhs | rhs;
	}

	inline PipelineStageType& operator&=(PipelineStageType& lhs, PipelineStageType rhs)
	{
		return lhs = lhs & rhs;
	}

	using PipelineShaderPathList = std::vector<std::pair<PipelineShaderStageType, const char*>>;

	struct PipelineShaderStage
	{
		public:
			PipelineShaderStageType stage_type{ PipelineShaderStageType::Vertex };
			ShaderID shader_module{ 0 };

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

	struct PushConstantPipelineData
	{
		int32_t offset{ 0 };
		size_t size{ 0 };
		PipelineShaderStageType shader_stage{ PipelineShaderStageType::Vertex };
		void* data;

		template<class T>
		static PushConstantPipelineData create(T* type_data, PipelineShaderStageType shader_stages)
		{
			PushConstantPipelineData push_constant_data;
			push_constant_data.size = sizeof(T);
			push_constant_data.data = type_data;
			push_constant_data.shader_stage = shader_stages;
			return push_constant_data;
		}

		bool operator==(const PushConstantPipelineData& other) const
		{
			return offset == other.offset
				&& size == other.size
				&& shader_stage == other.shader_stage
				&& data == other.data;
		}

		bool is_valid() const
		{
			return data != nullptr;
		}
	};

	enum class BlendFactor : uint32_t
	{
		Zero = 0,
		One,
		SourceColor,
		OneMinusSourceColor,
		DestinationColor,
		OneMinusDestinationColor,
		SourceAlpha,
		OneMinusSourceAlpha,
		DestinationAlpha,
		OneMinusDestinationAlpha
	};

	enum class BlendOp : uint32_t
	{
		Add = 0,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};

	struct PipelineAttachmentBlendState
	{
		bool b_disable_write{ false };
		bool b_blend_enabled{ true };
		BlendFactor source_color_blend{ BlendFactor::One };
		BlendFactor destination_color_blend{ BlendFactor::Zero };
		BlendOp color_blend_op{ BlendOp::Add };
		BlendFactor source_alpha_blend{ BlendFactor::One };
		BlendFactor destination_alpha_blend{ BlendFactor::Zero };
		BlendOp alpha_blend_op{ BlendOp::Add };

		bool operator==(const PipelineAttachmentBlendState& other) const
		{
			return b_disable_write == other.b_disable_write
				&& b_blend_enabled == other.b_blend_enabled
				&& source_color_blend == other.source_color_blend
				&& destination_color_blend == other.destination_color_blend
				&& color_blend_op == other.color_blend_op
				&& source_alpha_blend == other.source_alpha_blend
				&& destination_alpha_blend == other.destination_alpha_blend
				&& alpha_blend_op == other.alpha_blend_op;
		}
	};

	struct PipelineStateData
	{
		public:
			PipelineStateType type{ PipelineStateType::Graphics };
			std::vector<PipelineShaderStage> shader_stages;
			std::vector<Viewport> viewports;
			std::vector<Scissor> scissors;
			PipelineVertexInputDescription vertex_input_description;
			ShaderLayoutID layout{ 0 };
			PipelinePrimitiveTopologyType primitive_topology_type{ PipelinePrimitiveTopologyType::TriangleList };
			PipelineRasterizerState rasterizer_state{{}};
			PipelineAttachmentBlendState attachment_blend_state{{}};
			uint16_t multisample_count{ 1 };
			bool b_depth_test_enabled{ false };
			bool b_depth_write_enabled{ false };
			CompareOperation compare_op{ CompareOperation::Always };
			RenderPassID render_pass;
			PushConstantPipelineData push_constant_data;

			bool operator==(const PipelineStateData& other) const
			{
				return type == other.type
					&& shader_stages == other.shader_stages
					&& viewports == other.viewports
					&& scissors == other.scissors
					&& vertex_input_description == other.vertex_input_description
					&& layout == other.layout
					&& primitive_topology_type == other.primitive_topology_type
					&& rasterizer_state == other.rasterizer_state
					&& attachment_blend_state == other.attachment_blend_state
					&& multisample_count == other.multisample_count
					&& b_depth_test_enabled == other.b_depth_test_enabled
					&& b_depth_write_enabled == other.b_depth_write_enabled
					&& compare_op == other.compare_op
					&& render_pass == other.render_pass
					&& push_constant_data == other.push_constant_data;
			}
	};
}

#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4267)

template<>
struct std::hash<Sunset::PipelineAttachmentBlendState>
{
	std::size_t operator()(const Sunset::PipelineAttachmentBlendState& bd_data) const
	{
		std::size_t hash = Sunset::Maths::cantor_pair_hash(bd_data.b_blend_enabled, bd_data.b_disable_write);
		hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(bd_data.source_color_blend));
		hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(bd_data.destination_color_blend));
		hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(bd_data.color_blend_op));
		hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(bd_data.source_alpha_blend));
		hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(bd_data.destination_alpha_blend));
		hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(bd_data.alpha_blend_op));
		return hash;
	}
};

template<>
struct std::hash<Sunset::PushConstantPipelineData>
{
	std::size_t operator()(const Sunset::PushConstantPipelineData& pc_data) const
	{
		std::size_t hash = Sunset::Maths::cantor_pair_hash(pc_data.offset, static_cast<int32_t>(pc_data.size));
		hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(pc_data.shader_stage));
		hash = Sunset::Maths::cantor_pair_hash(hash, reinterpret_cast<uintptr_t>(pc_data.data));
		return hash;
	}
};

template<>
struct std::hash<Sunset::PipelineStateData>
{
	std::size_t operator()(const Sunset::PipelineStateData& psd) const
	{
		std::size_t type_seed = static_cast<int32_t>(psd.type);

		std::size_t ss_seed = psd.shader_stages.size();
		for (auto& i : psd.shader_stages)
		{
			std::size_t hash = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(i.stage_type), static_cast<int32_t>(i.shader_module));
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

		std::size_t attachment_blend_seed = std::hash<Sunset::PipelineAttachmentBlendState>{}(psd.attachment_blend_state);

		std::size_t pipeline_layout_seed = static_cast<int32_t>(psd.layout);

		std::size_t topology_seed = static_cast<int32_t>(psd.primitive_topology_type);

		std::size_t multisample_seed = static_cast<int32_t>(psd.multisample_count);

		std::size_t depth_test_seed = static_cast<int32_t>(psd.b_depth_test_enabled);

		std::size_t depth_write_seed = static_cast<int32_t>(psd.b_depth_write_enabled);

		std::size_t compare_op_seed = static_cast<int32_t>(psd.compare_op);

		std::size_t pass_seed = static_cast<int32_t>(psd.render_pass);

		std::size_t pc_data_seed = std::hash<Sunset::PushConstantPipelineData>{}(psd.push_constant_data);

		std::size_t final_hash = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(type_seed), static_cast<int32_t>(ss_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(viewport_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(scissors_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(vertex_input_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(attachment_blend_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(pipeline_layout_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(topology_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(rasterizer_state_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(multisample_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(depth_test_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(depth_write_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(compare_op_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(pass_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(pc_data_seed));

		return final_hash;
	}
};

#pragma warning( pop ) 