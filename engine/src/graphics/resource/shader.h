#pragma once

#include <common.h>
#include <graphics/resource/shader_types.h>
#include <graphics/resource/resource_cache.h>

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

	DEFINE_RESOURCE_CACHE(ShaderCache, ShaderID, Shader);
}
