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
		template <typename T>
		friend class Singleton;
	public:
		void initialize() { }
		void update() { }

		bool exists(Identity id) const
		{
			ResourceIDType resource_id = static_cast<ResourceIDType>(id.computed_hash);
			return cache.find(resource_id) != cache.end();
		}

		ResourceIDType fetch_or_add(Identity id, GraphicsContext* const gfx_context, bool& b_added, bool b_auto_delete_if_added = true)
		{
			ResourceIDType resource_id = static_cast<ResourceIDType>(id.computed_hash);
			if (cache.find(resource_id) == cache.end())
			{
				ResourceType* const new_resource = GlobalAssetPools<ResourceType>::get()->allocate();
				gfx_context->add_resource_deletion_execution([this, resource_id, gfx_context]()
				{
					remove_and_delete(gfx_context, resource_id);
				});
				cache.insert({ resource_id, new_resource });
				b_added = true;
			}
			return resource_id;
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

		void remove_and_delete(class GraphicsContext* const gfx_context, ResourceIDType id)
		{
			ResourceType* resource = fetch(id);
			cache.erase(id);
			resource->destroy(gfx_context);
			GlobalAssetPools<ResourceType>::get()->deallocate(resource);
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
	#define CACHE_FETCH(TypeName, ResourceID) TypeName##Cache::get()->fetch(ResourceID)
	#define CACHE_EXISTS(TypeName, ResourceID) TypeName##Cache::get()->exists(ResourceID)
	#define CACHE_DELETE(TypeName, ResourceID, Context) TypeName##Cache::get()->remove_and_delete(Context, ResourceID)
}