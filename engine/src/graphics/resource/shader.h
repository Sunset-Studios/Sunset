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

		bool is_compiled() const
		{
			return shader_policy.is_compiled();
		}

		size_t get_code_size()
		{
			return shader_policy.get_code_size();
		}

		uint32_t* get_code()
		{
			return shader_policy.get_code();
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

		void reflect_shader_layout(class GraphicsContext* const gfx_context)
		{ }

		void* get_data()
		{
			return nullptr;
		}

		ShaderLayoutID get_shader_layout()
		{
			return 0;
		}

		bool is_compiled() const
		{
			return false;
		}

		size_t get_code_size()
		{
			return 0;
		}

		uint32_t* get_code()
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
		static ShaderID create(class GraphicsContext* gfx_context, const char* shader_path);
	};

	DEFINE_RESOURCE_CACHE(ShaderCache, ShaderID, Shader);
}
