#pragma once

#include <vk_types.h>
#include <vk_initializers.h>

namespace Sunset
{
	struct VulkanShaderData
	{
		std::vector<uint32_t> shader_code;
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
			size_t get_code_size()
			{
				return data.shader_code.size() * sizeof(uint32_t);
			}
			uint32_t* get_code()
			{
				return data.shader_code.data();
			}

		protected:
			VulkanShaderData data;
	};
}
