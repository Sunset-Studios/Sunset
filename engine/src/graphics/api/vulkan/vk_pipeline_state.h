#pragma once

#include <vector>

#include <vk_types.h>

#include <graphics/viewport.h>
#include <pipeline_types.h>

namespace Sunset
{
	inline VkPrimitiveTopology VK_FROM_SUNSET_TOPOLOGY_TYPE(PipelinePrimitiveTopologyType topology_type)
	{
		switch (topology_type)
		{
			case PipelinePrimitiveTopologyType::LineList:
				return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case PipelinePrimitiveTopologyType::LineStrip:
				return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case PipelinePrimitiveTopologyType::PointList:
				return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case PipelinePrimitiveTopologyType::TriangleFan:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			case PipelinePrimitiveTopologyType::TriangleList:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case PipelinePrimitiveTopologyType::TriangleStrip:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			default:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	}

	inline VkPolygonMode VK_FROM_SUNSET_POLYGON_DRAW_TYPE(PipelineRasterizerPolygonMode polygon_type)
	{
		switch (polygon_type)
		{
			case PipelineRasterizerPolygonMode::Fill:
				return VK_POLYGON_MODE_FILL;
			case PipelineRasterizerPolygonMode::Line:
				return VK_POLYGON_MODE_LINE;
			case PipelineRasterizerPolygonMode::Point:
				return VK_POLYGON_MODE_POINT;
			default:
				return VK_POLYGON_MODE_FILL;
		}
	}

	inline VkCullModeFlags VK_FROM_SUNSET_CULL_MODE_TYPE(PipelineRasterizerCullMode cull_type)
	{
		switch (cull_type)
		{
			case PipelineRasterizerCullMode::None:
				return VK_CULL_MODE_NONE;
			case PipelineRasterizerCullMode::BackFace:
				return VK_CULL_MODE_BACK_BIT;
			case PipelineRasterizerCullMode::FrontFace:
				return VK_CULL_MODE_FRONT_BIT;
			case PipelineRasterizerCullMode::FrontAndBackFace:
				return VK_CULL_MODE_FRONT_AND_BACK;
			default:
				return VK_CULL_MODE_NONE;
		}
	}

	inline VkSampleCountFlagBits VK_FROM_SUNSET_MULTISAMPLE_COUNT(int16_t count)
	{
		return static_cast<VkSampleCountFlagBits>(log2(count) + 1);
	}

	inline std::vector<VkViewport> VK_FROM_SUNSET_VIEWPORT_LIST(const std::vector<Viewport>& viewports)
	{
		std::vector<VkViewport> vk_viewports(viewports.size(), VkViewport());
		for (size_t i = 0; i < viewports.size(); ++i)
		{
			const Viewport& viewport = viewports[i];
			VkViewport& vk_viewport = vk_viewports[i];
			vk_viewport.x = viewport.x;
			vk_viewport.y = viewport.y;
			vk_viewport.width = viewport.width;
			vk_viewport.height = viewport.height;
			vk_viewport.minDepth = viewport.min_depth;
			vk_viewport.maxDepth = viewport.max_depth;
		}
		return vk_viewports;
	}

	inline std::vector<VkRect2D> VK_FROM_SUNSET_SCISSORS_LIST(const std::vector<Scissor>& scissors)
	{
		std::vector<VkRect2D> vk_scissors(scissors.size(), VkRect2D());
		for (size_t i = 0; i < scissors.size(); ++i)
		{
			const Scissor& scissor = scissors[i];
			VkRect2D& vk_scissor = vk_scissors[i];
			vk_scissor.offset.x = scissor.x;
			vk_scissor.offset.y = scissor.y;
			vk_scissor.extent.width = scissor.w;
			vk_scissor.extent.height = scissor.h;
		}
		return vk_scissors;
	}

	inline std::vector<VkVertexInputBindingDescription> VK_FROM_SUNSET_VERTEX_BINDING_DESCRIPTION(const std::vector<VertexBinding>& bindings)
	{
		std::vector<VkVertexInputBindingDescription> vk_bindings;
		for (const VertexBinding& binding : bindings)
		{
			VkVertexInputBindingDescription binding_desc;
			binding_desc.binding = static_cast<uint32_t>(binding.index);
			binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			binding_desc.stride = static_cast<uint32_t>(binding.stride);
			vk_bindings.push_back(binding_desc);
		}
		return vk_bindings;
	}

	inline std::vector<VkVertexInputAttributeDescription> VK_FROM_SUNSET_VERTEX_ATTRIBUTE_DESCRIPTION(const std::vector<VertexAttribute>& attributes)
	{
		std::vector<VkVertexInputAttributeDescription> vk_attributes;
		for (const VertexAttribute& attribute : attributes)
		{
			VkVertexInputAttributeDescription attribute_desc;
			attribute_desc.binding = static_cast<uint32_t>(attribute.binding);
			attribute_desc.location = static_cast<uint32_t>(attribute.index);
			attribute_desc.format = VK_FROM_SUNSET_FORMAT(attribute.format);
			attribute_desc.offset = static_cast<uint32_t>(attribute.data_offset);
			vk_attributes.push_back(attribute_desc);
		}
		return vk_attributes;
	}

	inline VkCompareOp VK_FROM_SUNSET_COMPARE_OP(CompareOperation compare_op)
	{
		switch (compare_op)
		{
			case CompareOperation::Equal:
				return VK_COMPARE_OP_EQUAL;
			case CompareOperation::Greater:
				return VK_COMPARE_OP_GREATER;
			case CompareOperation::GreaterOrEqual:
				return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case CompareOperation::Less:
				return VK_COMPARE_OP_LESS;
			case CompareOperation::LessOrEqual:
				return VK_COMPARE_OP_LESS_OR_EQUAL;
			default:
				return VK_COMPARE_OP_ALWAYS;
		}
	}

	inline VkPipelineColorBlendAttachmentState VK_FROM_SUNSET_ATTACHMENT_BLEND_STATE(const PipelineAttachmentBlendState& blend_state)
	{
		VkPipelineColorBlendAttachmentState vk_blend_state;
		vk_blend_state.colorWriteMask = blend_state.b_disable_write ? 0 : VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		vk_blend_state.blendEnable = blend_state.b_blend_enabled;
		vk_blend_state.srcColorBlendFactor = VK_FROM_SUNSET_BLEND_FACTOR(blend_state.source_color_blend);
		vk_blend_state.dstColorBlendFactor = VK_FROM_SUNSET_BLEND_FACTOR(blend_state.destination_color_blend);
		vk_blend_state.colorBlendOp = VK_FROM_SUNSET_BLEND_OP(blend_state.color_blend_op);
		vk_blend_state.srcAlphaBlendFactor = VK_FROM_SUNSET_BLEND_FACTOR(blend_state.source_alpha_blend);
		vk_blend_state.dstAlphaBlendFactor = VK_FROM_SUNSET_BLEND_FACTOR(blend_state.destination_alpha_blend);
		vk_blend_state.alphaBlendOp = VK_FROM_SUNSET_BLEND_OP(blend_state.alpha_blend_op);
		return vk_blend_state;
	}

	class VulkanPipelineState
	{
	public:
		void initialize(class GraphicsContext* const gfx_context, struct PipelineStateData* state_data);
		void destroy(class GraphicsContext* const gfx_context, struct PipelineStateData* state_data);
		void build(class GraphicsContext* const gfx_context, struct PipelineStateData* state_data, class RenderPass* render_pass);
		void build_compute(class GraphicsContext* const gfx_context, struct PipelineStateData* state_data, void* render_pass_data);
		void bind(class GraphicsContext* const gfx_context, PipelineStateType type, void* buffer);

		void* get_handle()
		{
			return pipeline;
		}

	protected:
		VkPipelineViewportStateCreateInfo new_viewport_state_create_info(const std::vector<VkViewport>& viewports, const std::vector<VkRect2D>& scissors);
		VkPipelineShaderStageCreateInfo new_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shader);
		VkPipelineVertexInputStateCreateInfo new_vertex_input_state_create_info(const std::vector<VkVertexInputBindingDescription>& bindings, const std::vector<VkVertexInputAttributeDescription>& attributes);
		VkPipelineInputAssemblyStateCreateInfo new_input_assembly_create_info(VkPrimitiveTopology topology);
		VkPipelineRasterizationStateCreateInfo new_rasterization_state_create_info(VkPolygonMode polygon_draw_mode, float polygon_line_width, VkCullModeFlags cull_mode);
		VkPipelineMultisampleStateCreateInfo new_multisample_state_create_info(VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT);
		VkPipelineColorBlendAttachmentState  new_color_blend_attachment_state();
		VkPipelineColorBlendStateCreateInfo new_color_blending_state(std::vector<VkPipelineColorBlendAttachmentState>& color_blend_attachment_states);
		VkPipelineDepthStencilStateCreateInfo new_depth_stencil_state(bool b_depth_test = true, bool b_depth_write = true, VkCompareOp compare_op = VK_COMPARE_OP_LESS_OR_EQUAL);

	protected:
		VkPipeline pipeline{ nullptr };
	};
}
