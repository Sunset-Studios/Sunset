#include <graphics/api/vulkan/vk_shader_pipeline_layout.h>
#include <graphics/api/vulkan/vk_pipeline_state.h>
#include <graphics/graphics_context.h>
#include <graphics/descriptor.h>

namespace Sunset
{
	void VulkanShaderPipelineLayout::initialize(class GraphicsContext* const gfx_context, const std::vector<PushConstantPipelineData>& push_constant_data, const std::vector<DescriptorLayoutID> descriptor_layouts)
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

		std::vector<VkPushConstantRange> pc_ranges;
		{
			pc_ranges.reserve(push_constant_data.size());
			for (const PushConstantPipelineData& pc_data : push_constant_data)
			{
				if (pc_data.size > 0)
				{
					VkPushConstantRange& push_constant = pc_ranges.emplace_back();
					push_constant.offset = pc_data.offset;
					push_constant.size = static_cast<uint32_t>(pc_data.size);
					push_constant.stageFlags = VK_FROM_SUNSET_SHADER_STAGE_TYPE(pc_data.shader_stage);
				}
			}
			create_info.pushConstantRangeCount = pc_ranges.size();
			create_info.pPushConstantRanges = pc_ranges.data();
		}

		std::vector<VkDescriptorSetLayout> set_layouts;
		if (descriptor_layouts.size() > 0)
		{
			for (DescriptorLayoutID layout : descriptor_layouts)
			{
				DescriptorLayout* const layout_obj = CACHE_FETCH(DescriptorLayout, layout);
				set_layouts.push_back(static_cast<VkDescriptorSetLayout>(layout_obj->get()));
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
