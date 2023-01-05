#include <graphics/resource/shader.h>
#include <graphics/graphics_context.h>
#include <graphics/asset_pool.h>

namespace Sunset
{
	void ShaderCache::initialize()
	{
	}

	void ShaderCache::update()
	{
	}

	Sunset::ShaderID ShaderCache::fetch_or_add(const char* file_path, class GraphicsContext* const gfx_context /*= nullptr*/)
	{
		ShaderID id = std::hash<std::string>{}(file_path);
		if (cache.find(id) == cache.end())
		{
			Shader* const new_shader = GlobalAssetPools<Shader>::get()->allocate();
			new_shader->initialize(gfx_context, file_path);
			gfx_context->add_resource_deletion_execution([new_shader, gfx_context]()
			{
				new_shader->destroy(gfx_context);
				GlobalAssetPools<Shader>::get()->deallocate(new_shader);
			});
			cache.insert({ id, new_shader });
		}
		return id;
	}

	Sunset::Shader* ShaderCache::fetch(ShaderID id)
	{
		assert(cache.find(id) != cache.end());
		return cache[id];
	}

	void ShaderCache::remove(ShaderID id)
	{
		cache.erase(id);
	}

	void ShaderCache::destroy(class GraphicsContext* const gfx_context)
	{
		for (const std::pair<size_t, Shader*>& pair : cache)
		{
			pair.second->destroy(gfx_context);
		}
		cache.clear();
	}
}
