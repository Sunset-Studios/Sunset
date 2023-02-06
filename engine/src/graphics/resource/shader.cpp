#include <graphics/resource/shader.h>

namespace Sunset
{
	Sunset::ShaderID ShaderFactory::create(class GraphicsContext* gfx_context, const char* shader_path)
	{
		bool b_added{ false };
		const ShaderID new_shader = ShaderCache::get()->fetch_or_add(shader_path, gfx_context, b_added);
		if (b_added)
		{
			Shader* const shader = CACHE_FETCH(Shader, new_shader);
			if (!shader->is_compiled())
			{
				shader->initialize(gfx_context, shader_path);
			}
		}
		return new_shader;
	}
}
