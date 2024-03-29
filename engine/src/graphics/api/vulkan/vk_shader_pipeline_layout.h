#pragma once

#include <vk_types.h>
#include <pipeline_types.h>

namespace Sunset
{
	struct VulkanShaderPipelineLayoutData
	{
		VkPipelineLayout layout{ nullptr };
	};

	class VulkanShaderPipelineLayout
	{
		public:
			void initialize(class GraphicsContext* const gfx_context, const std::vector<PushConstantPipelineData>& push_constant_data = {}, const std::vector<DescriptorLayoutID> descriptor_layouts = {});
			void destroy(class GraphicsContext* const gfx_context);

			void* get_data()
			{
				return data.layout;
			}

		protected:
			VulkanShaderPipelineLayoutData data;
	};
}
