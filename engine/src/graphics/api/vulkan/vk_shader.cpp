#include <graphics/api/vulkan/vk_shader.h>
#include <graphics/graphics_context.h>

#include <fstream>

namespace Sunset
{
	void VulkanShader::initialize(GraphicsContext* const gfx_context, const char* file_path)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		std::ifstream file(file_path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			return;
		}

		size_t file_size = static_cast<size_t>(file.tellg());

		std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));

		file.seekg(0);

		file.read(reinterpret_cast<char*>(buffer.data()), file_size);

		file.close();

		VkShaderModuleCreateInfo shader_create_info = {};
		shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_create_info.pNext = nullptr;
		shader_create_info.codeSize = buffer.size() * sizeof(uint32_t);
		shader_create_info.pCode = buffer.data();

		VkShaderModule out_shader_module;
		if (vkCreateShaderModule(context_state->get_device(), &shader_create_info, nullptr, &out_shader_module) != VK_SUCCESS)
		{
			std::cout << "Vulkan shader error: could not create shader module at " << file_path << std::endl;
			return;
		}

		data.shader_module = out_shader_module;
	}

	void VulkanShader::destroy(GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		vkDestroyShaderModule(context_state->get_device(), data.shader_module, nullptr);
	}
}
