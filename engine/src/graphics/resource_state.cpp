#include <graphics/resource_state.h>
#include <graphics/asset_pool.h>
#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>

namespace Sunset
{
	Sunset::ResourceStateBuilder ResourceStateBuilder::create()
	{
		ResourceStateBuilder state_builder;
		state_builder.context = Renderer::get()->context();
		return state_builder;
	}

	Sunset::ResourceStateBuilder ResourceStateBuilder::create(const ResourceStateData& data)
	{
		ResourceStateBuilder state_builder;
		state_builder.context = Renderer::get()->context();
		state_builder.state_data = data;
		return state_builder;
	}

	Sunset::ResourceStateBuilder& ResourceStateBuilder::set_vertex_buffer(class Buffer* buffer)
	{
		state_data.vertex_buffer = buffer;
		return *this;
	}

	Sunset::ResourceStateBuilder& ResourceStateBuilder::set_instance_index(uint32_t index)
	{
		state_data.instance_index = index;
		return *this;
	}

	Sunset::ResourceStateID ResourceStateBuilder::finish()
	{
		return ResourceStateCache::get()->fetch_or_add(state_data, context);
	}

	void ResourceStateCache::initialize()
	{
	}

	void ResourceStateCache::update()
	{
	}

	Sunset::ResourceStateID ResourceStateCache::fetch_or_add(const ResourceStateData& data, class GraphicsContext* const gfx_context /*= nullptr*/)
	{
		ResourceStateID id = std::hash<ResourceStateData>{}(data);
		if (cache.find(id) == cache.end())
		{
			ResourceState* const new_resource_state = GlobalAssetPools<ResourceState>::get()->allocate();
			new_resource_state->initialize(gfx_context, data);
			cache.insert({ id, new_resource_state });
		}
		return id;
	}

	void ResourceStateCache::remove(ResourceStateID id)
	{
		cache.erase(id);
	}

	Sunset::ResourceState* ResourceStateCache::fetch(ResourceStateID id)
	{
		assert(cache.find(id) != cache.end());
		return cache[id];
	}


	void ResourceStateCache::destroy(class GraphicsContext* const gfx_context)
	{
		cache.clear();
	}

	void ResourceState::bind(class GraphicsContext* const gfx_context, void* buffer)
	{
		assert(state_data.vertex_buffer != nullptr && "Cannot bind resource state with null vertex buffer. Make sure to properly set up the resource state data first.");
		state_data.vertex_buffer->bind(gfx_context, buffer);
	}
}
