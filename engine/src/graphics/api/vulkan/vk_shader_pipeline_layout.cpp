#include <graphics/api/vulkan/vk_shader_pipeline_layout.h>
#include <graphics/api/vulkan/vk_pipeline_state.h>
#include <graphics/graphics_context.h>
#include <graphics/descriptor.h>

namespace Sunset
{
	void VulkanShaderPipelineLayout::initialize(class GraphicsContext* const gfx_context, const PushConstantPipelineData& push_constant_data, const std::vector<DescriptorLayout*> descriptor_layouts)
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

		VkPushConstantRange push_constant;
		if (push_constant_data.size > 0)
		{
			push_constant.offset = push_constant_data.offset;
			push_constant.size = static_cast<uint32_t>(push_constant_data.size);
			push_constant.stageFlags = VK_FROM_SUNSET_SHADER_STAGE_TYPE(push_constant_data.shader_stage);

			create_info.pushConstantRangeCount = 1;
			create_info.pPushConstantRanges = &push_constant;
		}

		std::vector<VkDescriptorSetLayout> set_layouts;
		if (descriptor_layouts.size() > 0)
		{
			for (DescriptorLayout* const layout : descriptor_layouts)
			{
				set_layouts.push_back(static_cast<VkDescriptorSetLayout>(layout->get()));
			}

			create_info.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
			create_info.pSetLayouts = set_layouts.data();
		}

		VK_CHECK(vkCreatePipelineLayout(context_state->get_device(), &create_info, nullptr, &data.layout));
	}

	void VulkanShaderPipelineLayout::destroy(class GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		vkDestroyPipelineLayout(context_state->get_device(), data.layout, nullptr);
	}
}
