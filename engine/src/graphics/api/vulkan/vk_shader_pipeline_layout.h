#pragma once

#include <vk_types.h>
#include <vk_initializers.h>
#include <push_constants.h>

namespace Sunset
{
	struct VulkanShaderPipelineLayoutData
	{
		VkPipelineLayout layout{ nullptr };
	};

	class VulkanShaderPipelineLayout
	{
		public:
			void initialize(class GraphicsContext* const gfx_context, const PushConstantPipelineData& push_constant_data = {});
			void destroy(class GraphicsContext* const gfx_context);

			void* get_data()
			{
				return data.layout;
			}

		protected:
			VulkanShaderPipelineLayoutData data;
	};
}
