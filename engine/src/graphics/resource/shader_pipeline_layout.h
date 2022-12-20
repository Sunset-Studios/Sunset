#pragma once

#include <common.h>
#include <push_constants.h>

namespace Sunset
{
	template<class Policy>
	class GenericShaderPipelineLayout
	{
	public:
		GenericShaderPipelineLayout() = default;

		void initialize(class GraphicsContext* const gfx_context, const PushConstantPipelineData& push_constant_data = {}, const std::vector<class DescriptorLayout*> descriptor_layouts = {})
		{
			if (!b_initialized)
			{
				shader_layout_policy.initialize(gfx_context, push_constant_data, descriptor_layouts);
				b_initialized = true;
			}
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			shader_layout_policy.destroy(gfx_context);
		}

		void* get_data()
		{
			return shader_layout_policy.get_data();
		}

	private:
		Policy shader_layout_policy;
		bool b_initialized{ false };
	};

	class NoopShaderPipelineLayout
	{
	public:
		NoopShaderPipelineLayout() = default;

		void initialize(class GraphicsContext* const gfx_context, const PushConstantPipelineData& push_constant_data = {}, const std::vector<class DescriptorLayout*> descriptor_layouts = {})
		{ }

		void destroy(class GraphicsContext* const gfx_context)
		{ }

		void* get_data()
		{
			return nullptr;
		}
	};

#if USE_VULKAN_GRAPHICS
	class ShaderPipelineLayout : public GenericShaderPipelineLayout<VulkanShaderPipelineLayout>
	{ };
#else
	class ShaderPipelineLayout : public GenericShaderPipelineLayout<NoopShaderPipelineLayout>
	{ };
#endif

	class ShaderPipelineLayoutFactory
	{
	public:
		static ShaderPipelineLayout* get_default(class GraphicsContext* const gfx_context);
		static ShaderPipelineLayout* create(class GraphicsContext* const gfx_context, const PushConstantPipelineData& push_constant_data = {}, const std::vector<class DescriptorLayout*> descriptor_layouts = {});
	};
}
