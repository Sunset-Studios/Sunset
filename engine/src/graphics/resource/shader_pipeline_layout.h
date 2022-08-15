#pragma once

#include <common.h>

namespace Sunset
{
	template<class Policy>
	class GenericShaderPipelineLayout
	{
	public:
		GenericShaderPipelineLayout() = default;

		void initialize(class GraphicsContext* const gfx_context)
		{
			shader_layout_policy.initialize(gfx_context);
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
	};

	class NoopShaderPipelineLayout
	{
	public:
		NoopShaderPipelineLayout() = default;

		void initialize(class GraphicsContext* const gfx_context)
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
		template<typename ...Args>
		static ShaderPipelineLayout* create(Args&&... args)
		{
			ShaderPipelineLayout* layout = new ShaderPipelineLayout;
			layout->initialize(std::forward<Args>(args)...);
			return layout;
		}
	};
}
