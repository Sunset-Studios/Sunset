#pragma once

#include <minimal.h>
#include <graphics/graphics_context.h>
#include <graphics/asset_pool.h>
#include <utility/pattern/singleton.h>
#include <utility/strings.h>

namespace Sunset
{
	template <typename ResourceIDType, typename ResourceType>
	class ResourceCache : public Singleton<ResourceCache<ResourceIDType, ResourceType>>
	{
	public:
		void initialize() { }
		void update() { }

		ResourceIDType fetch_or_add(Identity id, GraphicsContext* const gfx_context /*= nullptr*/)
		{
			ResourceIDType id = static_cast<ResourceIDType>(id.computed_hash);
			if (cache.find(id) == cache.end())
			{
				ResourceType* const new_resource = GlobalAssetPools<ResourceType>::get()->allocate();
				gfx_context->add_resource_deletion_execution([id, new_resource, gfx_context]()
					{
						cache.erase(id);
						new_resource->destroy(gfx_context);
						GlobalAssetPools<ResourceType>::get()->deallocate(new_resource);
					});
				cache.insert({ id, new_resource });
			}
			return id;
		}

		ResourceType* fetch(ResourceIDType id)
		{
			assert(cache.find(id) != cache.end());
			return cache[id];
		}

		void remove(ResourceIDType id)
		{
			cache.erase(id);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			for (const std::pair<ResourceIDType, ResourceType*>& pair : cache)
			{
				pair.second->destroy(gfx_context);
			}
			cache.clear();
		}

		size_t size() const
		{
			return cache.size();
		}

	protected:
		std::unordered_map<ResourceIDType, ResourceType*> cache;

	private:
		ResourceCache() = default;
	};

	#define DEFINE_RESOURCE_CACHE(CacheName, ResourceIDType, ResourceType) class CacheName : public ResourceCache<ResourceIDType, ResourceType> { }
	#define CACHE_FETCH(Name, ID) Name##Cache::get()->fetch(ID)
}