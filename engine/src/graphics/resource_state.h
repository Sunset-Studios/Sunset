#pragma once

#include <singleton.h>
#include <resource_types.h>

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

			void bind(class GraphicsContext* const gfx_context, void* buffer);

		public:
			ResourceStateData state_data;
	};

	class ResourceStateBuilder
	{
	public:
		static ResourceStateBuilder create();
		static ResourceStateBuilder create(const ResourceStateData& data);

		ResourceStateBuilder& set_vertex_buffer(class Buffer* buffer);

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

	class ResourceStateCache : public Singleton<ResourceStateCache>
	{
		friend class Singleton;

	public:
		void initialize();
		void update();

		ResourceStateID fetch_or_add(const ResourceStateData& data, class GraphicsContext* const gfx_context = nullptr);
		void remove(ResourceStateID id);
		ResourceState* fetch(ResourceStateID id);
		void destroy(class GraphicsContext* const gfx_context);

		size_t size() const
		{
			return cache.size();
		}

	protected:
		std::unordered_map<ResourceStateID, ResourceState*> cache;

	private:
		ResourceStateCache() = default;
	};
}
