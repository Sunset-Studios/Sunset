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

	Sunset::ResourceStateBuilder& ResourceStateBuilder::set_index_buffer(class Buffer* buffer)
	{
		state_data.index_buffer = buffer;
		return *this;
	}

	Sunset::ResourceStateBuilder& ResourceStateBuilder::set_instance_index(uint32_t index)
	{
		state_data.instance_index = index;
		return *this;
	}

	Sunset::ResourceStateBuilder& ResourceStateBuilder::set_vertex_count(uint32_t count)
	{
		state_data.vertex_count = count;
		return *this;
	}

	Sunset::ResourceStateBuilder& ResourceStateBuilder::set_index_count(uint32_t count)
	{
		state_data.index_count = count;
		return *this;
	}

	Sunset::ResourceStateID ResourceStateBuilder::finish()
	{
		Identity cache_id = std::hash<ResourceStateData>{}(state_data);
		ResourceStateID resource_state_id = ResourceStateCache::get()->fetch_or_add(cache_id, context);
		ResourceState* const resource_state = ResourceStateCache::get()->fetch(resource_state_id);
		resource_state->initialize(context, state_data);
		return resource_state_id;
	}

	void ResourceState::bind(class GraphicsContext* const gfx_context, void* buffer)
	{
		assert(state_data.vertex_buffer != nullptr && "Cannot bind resource state with null vertex buffer. Make sure to properly set up the resource state data first.");
		state_data.vertex_buffer->bind(gfx_context, buffer);
		state_data.index_buffer->bind(gfx_context, buffer);
	}
}
