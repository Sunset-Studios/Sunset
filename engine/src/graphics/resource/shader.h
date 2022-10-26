#pragma once

#include <common.h>
#include <singleton.h>
#include <graphics/resource/shader_types.h>

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

	class ShaderCache : public Singleton<ShaderCache>
	{
		friend class Singleton;

	public:
		void initialize();
		void update();

		ShaderID fetch_or_add(const char* file_path, class GraphicsContext* const gfx_context = nullptr);
		Shader* fetch(ShaderID id);
		void remove(ShaderID id);
		void destroy(class GraphicsContext* const gfx_context);

		size_t size() const
		{
			return cache.size();
		}

	protected:
		std::unordered_map<ShaderID, Shader*> cache;

	private:
		ShaderCache() = default;
	};
}
