#pragma once

#include <resource_types.h>
#include <resource_cache.h>

namespace Sunset
{
	class ResourceState
	{
		public:
			ResourceState() = default;
			~ResourceState() = default;

			void initialize(class GraphicsContext* const gfx_context, const ResourceStateData& data)
			{
				state_data = data;
			}

			void destroy(class GraphicsContext* const gfx_context)
			{ }

			void bind(class GraphicsContext* const gfx_context, void* buffer);

		public:
			ResourceStateData state_data;
	};

	class ResourceStateBuilder
	{
	public:
		static ResourceStateBuilder create();
		static ResourceStateBuilder create(const ResourceStateData& data);

		ResourceStateBuilder& set_vertex_buffer(BufferID buffer);
		ResourceStateBuilder& set_index_buffer(BufferID buffer);
		ResourceStateBuilder& set_vertex_count(uint32_t count);
		ResourceStateBuilder& set_index_count(uint32_t count);

		ResourceStateID finish();

		ResourceStateBuilder value() const
		{
			return *this;
		}

		ResourceStateID get_state() const
		{
			return resource_state;
		}

	protected:
		ResourceStateData state_data;
		ResourceStateID resource_state;
		class GraphicsContext* context{ nullptr };
	};

	DEFINE_RESOURCE_CACHE(ResourceStateCache, ResourceStateID, ResourceState);
}
