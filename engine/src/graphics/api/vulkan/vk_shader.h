#pragma once

#include <vk_types.h>
#include <vk_initializers.h>

namespace Sunset
{
	struct VulkanShaderData
	{
		VkShaderModule shader_module{ nullptr };
	};

	class VulkanShader
	{
		public:
			void initialize(class GraphicsContext* const gfx_context, const char* file_path);
			void destroy(class GraphicsContext* const gfx_context);
			void* get_data()
			{
				return &data;
			}

		protected:
			VulkanShaderData data;
	};
}
