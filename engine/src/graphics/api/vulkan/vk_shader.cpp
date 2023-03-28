#include <graphics/api/vulkan/vk_shader.h>
#include <graphics/graphics_context.h>
#include <shader_serializer.h>

namespace Sunset
{
	void VulkanShader::initialize(GraphicsContext* const gfx_context, const char* file_path)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		SerializedAsset asset;
		if (!deserialize_asset(file_path, asset))
		{
			// TODO: Need some custom logging
			return;
		}

		SerializedShaderInfo serialized_shader_info = get_serialized_shader_info(&asset);

		std::vector<uint32_t> buffer(serialized_shader_info.shader_buffer_size / sizeof(uint32_t));

		unpack_shader(&serialized_shader_info, asset.binary.data(), asset.binary.size(), (char*)buffer.data());

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

		data.shader_code = std::move(buffer);
		data.shader_module = out_shader_module;
		data.shader_path = file_path;
	}

	void VulkanShader::destroy(GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		vkDestroyShaderModule(context_state->get_device(), data.shader_module, nullptr);
	}
}
