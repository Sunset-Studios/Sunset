#pragma once

#include <vk_types.h>
#include <vk_initializers.h>

namespace Sunset
{
	struct VulkanShaderPipelineLayoutData
	{
		VkPipelineLayout layout{ nullptr };
	};

	class VulkanShaderPipelineLayout
	{
		public:
			void initialize(class GraphicsContext* const gfx_context);
			void destroy(class GraphicsContext* const gfx_context);

			void* get_data()
			{
				return &data;
			}

		protected:
			VulkanShaderPipelineLayoutData data;
	};
}
