#pragma once

#include <common.h>

namespace Sunset
{
	template<class Policy>
	class GenericShader
	{
	public:
		GenericShader() = default;

		void initialize(class GraphicsContext* const gfx_context, const char* file_path)
		{
			shader_policy.initialize(gfx_context, file_path);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			shader_policy.destroy(gfx_context);
		}

		void* get_data()
		{
			return shader_policy.get_data();
		}

	private:
		Policy shader_policy;
	};

	class NoopShader
	{
	public:
		NoopShader() = default;

		void initialize(class GraphicsContext* const gfx_context, const char* file_path)
		{ }

		void destroy(class GraphicsContext* const gfx_context)
		{ }

		void* get_data()
		{
			return nullptr;
		}
	};

#if USE_VULKAN_GRAPHICS
	class Shader : public GenericShader<VulkanShader>
	{ };
#else
	class Shader : public GenericShader<NoopShader>
	{ };
#endif

	class ShaderFactory
	{
	public:
		template<typename ...Args>
		static Shader* create(Args&&... args)
		{
			Shader* shader = new Shader;
			shader->initialize(std::forward<Args>(args)...);
			return shader;
		}
	};
}
