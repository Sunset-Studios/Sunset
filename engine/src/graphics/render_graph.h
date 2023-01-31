#pragma once

#include <common.h>

#include <graphics/render_task.h>
#include <graphics/resource/image_types.h>
#include <graphics/resource/buffer_types.h>
#include <graphics/render_pass_types.h>

namespace Sunset
{
	using RGPassHandle = uint32_t;
	using RGResourceHandle = uint64_t;

	using RGResourceIndex = uint32_t;
	using RGResourceVersion = uint16_t;
	using RGResourceType = uint16_t;

	inline RGResourceHandle create_graph_resource_handle(RGResourceIndex index, RGResourceType type, RGResourceVersion version)
	{
		return ((RGResourceIndex)index << 32) | ((RGResourceType)type << 16) | ((RGResourceVersion)version);
	}

	inline RGResourceIndex get_graph_resource_index(RGResourceHandle handle)
	{
		return handle >> 32;
	}

	inline RGResourceType get_graph_resource_type(RGResourceHandle handle)
	{
		return (RGResourceType)(handle >> 16);
	}

	inline RGResourceVersion get_graph_resource_version(RGResourceHandle handle)
	{
		return (RGResourceVersion)handle;
	}

	inline bool is_valid_graph_resource(RGResourceHandle handle)
	{
		return (handle >> 32) != RGResourceIndex(-1);
	}

#define INVALID_GRAPH_RESOURCE create_graph_resource_handle(RGResourceIndex(-1), 0)

	template <typename ResourceConfig>
	class RGResource
	{
	protected:
		ResourceConfig config;
		int32_t reference_count{ 0 };
		int32_t physical_resource_id{ 0 };
	};

	class RGImageResource : RGResource<AttachmentConfig>
	{
		friend class RenderGraph;
	};

	class RGBufferResource : RGResource<BufferConfig>
	{
		friend class RenderGraph;
	};

	struct RGPassParameters
	{
		std::vector<RGResourceHandle> inputs;
		std::vector<RGResourceHandle> outputs;
	};

	class RGPass
	{
		friend class RenderGraph;
	protected:
		RenderPassConfig pass_config;
		RGPassParameters parameters;
		int32_t physical_pass_id{ 0 };
	};

	class RenderGraph
	{
	public:
		RenderGraph() = default;
		~RenderGraph() = default;

		void initialize(class GraphicsContext* const gfx_context);
		void destroy(class GraphicsContext* const gfx_context);

		void compile(class GraphicsContext* const gfx_context);
		void submit(class GraphicsContext* const gfx_context, class Swapchain* const swapchain);
		void reset();

	protected:
		RenderTaskFrameAllocator task_allocator;

		std::vector<RGImageResource*> image_resource_registry;
		std::vector<RGBufferResource*> buffer_resource_registry;
		std::vector<RGPass*> registered_passes;
		std::vector<RGPassHandle> nonculled_passes;
	};
}
