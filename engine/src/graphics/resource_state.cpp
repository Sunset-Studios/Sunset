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

	Sunset::ResourceStateBuilder& ResourceStateBuilder::set_vertex_buffer(BufferID buffer)
	{
		state_data.vertex_buffer = buffer;
		return *this;
	}

	Sunset::ResourceStateBuilder& ResourceStateBuilder::set_index_buffer(BufferID buffer)
	{
		state_data.index_buffer = buffer;
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
		bool b_state_added{ false };
		Identity cache_id = std::hash<ResourceStateData>{}(state_data);
		ResourceStateID resource_state_id = ResourceStateCache::get()->fetch_or_add(cache_id, context, b_state_added);
		if (b_state_added)
		{
			ResourceState* const resource_state = CACHE_FETCH(ResourceState, resource_state_id);
			resource_state->initialize(context, state_data);
		}
		return resource_state_id;
	}

	void ResourceState::bind(class GraphicsContext* const gfx_context, void* buffer)
	{
		assert(state_data.vertex_buffer != 0 && state_data.index_buffer != 0 && "Cannot bind resource state with null vertex and/or index buffer. Make sure to properly set up the resource state data first.");

		CACHE_FETCH(Buffer, state_data.vertex_buffer)->bind(gfx_context, buffer);
		CACHE_FETCH(Buffer, state_data.index_buffer)->bind(gfx_context, buffer);
	}
}
