#include <graphics/api/vulkan/vk_shader_pipeline_layout.h>
#include <graphics/api/vulkan/vk_pipeline_state.h>
#include <graphics/graphics_context.h>

namespace Sunset
{
	void VulkanShaderPipelineLayout::initialize(class GraphicsContext* const gfx_context, const PushConstantPipelineData& push_constant_data)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		VkPipelineLayoutCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.setLayoutCount = 0;
		create_info.pSetLayouts = nullptr;
		create_info.pushConstantRangeCount = 0;
		create_info.pPushConstantRanges = nullptr;

		if (push_constant_data.size > 0)
		{
			VkPushConstantRange push_constant;
			push_constant.offset = push_constant_data.offset;
			push_constant.size = static_cast<uint32_t>(push_constant_data.size);
			push_constant.stageFlags = VK_FROM_SUNSET_SHADER_STAGE_TYPE(push_constant_data.shader_stage);

			create_info.pushConstantRangeCount = 1;
			create_info.pPushConstantRanges = &push_constant;
		}

		VK_CHECK(vkCreatePipelineLayout(context_state->get_device(), &create_info, nullptr, &data.layout));
	}

	void VulkanShaderPipelineLayout::destroy(class GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		vkDestroyPipelineLayout(context_state->get_device(), data.layout, nullptr);
	}
}
