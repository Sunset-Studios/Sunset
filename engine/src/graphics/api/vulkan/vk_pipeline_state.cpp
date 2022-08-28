#include <graphics/api/vulkan/vk_pipeline_state.h>
#include <graphics/pipeline_state.h>
#include <graphics/resource/shader.h>
#include <graphics/render_pass.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/shader_pipeline_layout.h>

namespace Sunset
{
	void VulkanPipelineState::initialize(class GraphicsContext* const gfx_context, PipelineStateData* state_data)
	{

	}

	void VulkanPipelineState::destroy(class GraphicsContext* const gfx_context, PipelineStateData* state_data)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		vkDestroyPipeline(context_state->get_device(), pipeline, nullptr);

		for (PipelineShaderStage& shader_stage : state_data->shader_stages)
		{
			shader_stage.shader_module->destroy(gfx_context);
		}
	}

	void VulkanPipelineState::build(class GraphicsContext* const gfx_context, PipelineStateData* state_data, void* render_pass_data)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		VulkanRenderPassData* render_pass = static_cast<VulkanRenderPassData*>(render_pass_data);
		VulkanShaderPipelineLayoutData* pipeline_layout_data = static_cast<VulkanShaderPipelineLayoutData*>(state_data->layout->get_data());

		std::vector<VkViewport> viewports(VK_FROM_SUNSET_VIEWPORT_LIST(state_data->viewports));
		std::vector<VkRect2D> scissors(VK_FROM_SUNSET_SCISSORS_LIST(state_data->scissors));

		VkPipelineViewportStateCreateInfo viewport_state = new_viewport_state_create_info(viewports, scissors);

		std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
		for (PipelineShaderStage& shader_stage : state_data->shader_stages)
		{
			VulkanShaderData* shader_data = static_cast<VulkanShaderData*>(shader_stage.shader_module->get_data());
			shader_stages.push_back(new_shader_stage_create_info(VK_FROM_SUNSET_SHADER_STAGE_TYPE(shader_stage.stage_type), shader_data->shader_module));
		}

		VkPipelineVertexInputStateCreateInfo vertex_input_state = new_vertex_input_state_create_info();
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state = new_input_assembly_create_info(VK_FROM_SUNSET_TOPOLOGY_TYPE(state_data->primitive_topology_type));
		VkPipelineRasterizationStateCreateInfo rasterization_state = new_rasterization_state_create_info(
			VK_FROM_SUNSET_POLYGON_DRAW_TYPE(state_data->rasterizer_state.polygon_draw_mode), state_data->rasterizer_state.line_width, VK_FROM_SUNSET_CULL_MODE_TYPE(state_data->rasterizer_state.cull_mode)
		);
		VkPipelineMultisampleStateCreateInfo multisample_state = new_multisample_state_create_info(VK_FROM_SUNSET_MULTISAMPLE_COUNT(state_data->multisample_count));
		VkPipelineColorBlendAttachmentState color_blend_attachment_state = new_color_blend_attachment_state();
		VkPipelineColorBlendStateCreateInfo color_blending_state = new_color_blending_state(color_blend_attachment_state);

		VkGraphicsPipelineCreateInfo pipeline_create_info = {};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_create_info.pNext = nullptr;
		pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
		pipeline_create_info.pStages = shader_stages.data();
		pipeline_create_info.pVertexInputState = &vertex_input_state;
		pipeline_create_info.pInputAssemblyState = &input_assembly_state;
		pipeline_create_info.pViewportState = &viewport_state;
		pipeline_create_info.pRasterizationState = &rasterization_state;
		pipeline_create_info.pMultisampleState = &multisample_state;
		pipeline_create_info.pColorBlendState = &color_blending_state;
		pipeline_create_info.layout = pipeline_layout_data->layout;
		pipeline_create_info.renderPass = render_pass->render_pass;
		pipeline_create_info.subpass = 0;
		pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;

		VkPipeline new_pipeline;
		if (vkCreateGraphicsPipelines(context_state->get_device(), VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &new_pipeline) != VK_SUCCESS)
		{
			std::cout << "Vulkan pipeline error: Failed to create graphics pipeline" << std::endl;
			return;
		}

		pipeline = new_pipeline;
	}

	void VulkanPipelineState::bind(class GraphicsContext* const gfx_context, void* buffer)
	{
		vkCmdBindPipeline(static_cast<VkCommandBuffer>(buffer), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	VkPipelineViewportStateCreateInfo VulkanPipelineState::new_viewport_state_create_info(const std::vector<VkViewport>& viewports, const std::vector<VkRect2D>& scissors)
	{
		VkPipelineViewportStateCreateInfo viewport_state = {};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.pNext = nullptr;
		viewport_state.viewportCount = static_cast<uint32_t>(viewports.size());
		viewport_state.pViewports = viewports.data();
		viewport_state.scissorCount = static_cast<uint32_t>(scissors.size());
		viewport_state.pScissors = scissors.data();
		return viewport_state;
	}

	VkPipelineShaderStageCreateInfo VulkanPipelineState::new_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shader)
	{
		VkPipelineShaderStageCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.stage = stage;
		create_info.module = shader;
		create_info.pName = "main";
		return create_info;
	}


	VkPipelineVertexInputStateCreateInfo VulkanPipelineState::new_vertex_input_state_create_info()
	{
		VkPipelineVertexInputStateCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.vertexBindingDescriptionCount = 0;
		create_info.vertexAttributeDescriptionCount = 0;
		return create_info;
	}

	VkPipelineInputAssemblyStateCreateInfo VulkanPipelineState::new_input_assembly_create_info(VkPrimitiveTopology topology)
	{
		VkPipelineInputAssemblyStateCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.topology = topology;
		create_info.primitiveRestartEnable = VK_FALSE;
		return create_info;
	}

	VkPipelineRasterizationStateCreateInfo VulkanPipelineState::new_rasterization_state_create_info(VkPolygonMode polygon_draw_mode, float polygon_line_width, VkCullModeFlags cull_mode)
	{
		VkPipelineRasterizationStateCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.depthClampEnable = VK_FALSE;
		create_info.rasterizerDiscardEnable = VK_FALSE;
		create_info.polygonMode = polygon_draw_mode;
		create_info.lineWidth = polygon_line_width;
		create_info.cullMode = cull_mode;
		create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		create_info.depthBiasEnable = VK_FALSE;
		create_info.depthBiasConstantFactor = 0.0f;
		create_info.depthBiasClamp = 0.0f;
		create_info.depthBiasSlopeFactor = 0.0f;
		return create_info;
	}

	VkPipelineMultisampleStateCreateInfo VulkanPipelineState::new_multisample_state_create_info(VkSampleCountFlagBits sample_count)
	{
		VkPipelineMultisampleStateCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.sampleShadingEnable = !(sample_count & VK_SAMPLE_COUNT_1_BIT);
		create_info.rasterizationSamples = sample_count;
		create_info.minSampleShading = 1.0f;
		create_info.pSampleMask = nullptr;
		create_info.alphaToCoverageEnable = VK_FALSE;
		create_info.alphaToOneEnable = VK_FALSE;
		return create_info;
	}

	VkPipelineColorBlendAttachmentState VulkanPipelineState::new_color_blend_attachment_state()
	{
		VkPipelineColorBlendAttachmentState color_blend_attachment = {};
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_FALSE;
		return color_blend_attachment;
	}

	VkPipelineColorBlendStateCreateInfo VulkanPipelineState::new_color_blending_state(VkPipelineColorBlendAttachmentState& color_blend_attachment_state)
	{
		VkPipelineColorBlendStateCreateInfo color_blending_state = {};
		color_blending_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending_state.pNext = nullptr;
		color_blending_state.logicOpEnable = VK_FALSE;
		color_blending_state.logicOp = VK_LOGIC_OP_COPY;
		color_blending_state.attachmentCount = 1;
		color_blending_state.pAttachments = &color_blend_attachment_state;
		return color_blending_state;
	}

}
