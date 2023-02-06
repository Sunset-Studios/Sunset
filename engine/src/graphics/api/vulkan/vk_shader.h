#pragma once

#include <vk_types.h>
#include <vk_initializers.h>

namespace Sunset
{
	struct VulkanShaderData
	{
		VkShaderModule shader_module{ nullptr };
		const char* shader_path{ nullptr };
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
			bool is_compiled() const
			{
				return data.shader_module != nullptr;
			}

		protected:
			VulkanShaderData data;
	};
}
