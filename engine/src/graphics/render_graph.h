#pragma once

#include <common.h>

#include <graphics/render_task.h>
#include <graphics/resource/image_types.h>
#include <graphics/resource/buffer_types.h>
#include <graphics/render_pass_types.h>

#include <unordered_set>

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
		RGResourceHandle handle;
		ResourceConfig config;
		int32_t physical_id{ 0 };
	};

	class RGImageResource : RGResource<AttachmentConfig>
	{
		friend class RenderGraph;
	};

	class RGBufferResource : RGResource<BufferConfig>
	{
		friend class RenderGraph;
	};

	struct RGResourceMetadata
	{
		int32_t reference_count{ 0 };
		std::vector<RGPassHandle> producers;
		std::vector<RGPassHandle> consumers;
		RGPassHandle first_user;
		RGPassHandle last_user;
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
		RGPassHandle handle;
		RenderPassConfig pass_config;
		RGPassParameters parameters;
		std::function<void(class RenderGraph&, void*)> executor;
		int32_t reference_count{ 0 };
		int32_t physical_id{ 0 };
	};

	using ImageResourceFrameAllocator = StaticFrameAllocator<RGImageResource, 256>;
	using BufferResourceFrameAllocator = StaticFrameAllocator<RGBufferResource, 256>;
	using RenderPassFrameAllocator = StaticFrameAllocator<RGPass, 128>;

	// All render graph registry resources (aside from render passes) should be transient and therefore do not need serious caching.
	// Any resource that need to survive multiple frames should be allocated externally and registered to the render graph as external
	// resources.
	class RenderGraphRegistry
	{
		friend class RenderGraph;
	protected:
		std::vector<RGImageResource*> image_resources;
		std::vector<RGBufferResource*> buffer_resources;
		std::vector<RGPass*> render_passes;
		std::vector<RGResourceHandle> all_resource_handles;
		std::unordered_map<RGResourceHandle, RGResourceMetadata> resource_metadata;
		std::unordered_set<Identity> pass_cache;
	};

	class RenderGraph
	{
	public:
		RenderGraph() = default;
		~RenderGraph() = default;

		void initialize(class GraphicsContext* const gfx_context);
		void destroy(class GraphicsContext* const gfx_context);

		RGResourceHandle create_image(
			class GraphicsContext* const gfx_context,
			const AttachmentConfig& config);

		RGResourceHandle create_buffer(
			class GraphicsContext* const gfx_context,
			const BufferConfig& config);

		RGPassHandle add_pass(
			class GraphicsContext* const gfx_context,
			Identity name,
			RenderPassFlags pass_type,
			const RGPassParameters& params,
			std::function<void(RenderGraph&, void*)> execution_callback);

		void submit(class GraphicsContext* const gfx_context, class Swapchain* const swapchain);

	protected:
		void update_reference_counts(RGPass* pass);
		void update_resource_param_producers_and_consumers(RGPass* pass);
		void cull_graph_passes(class GraphicsContext* const gfx_context);
		void compute_resource_first_and_last_users(class GraphicsContext* const gfx_context);
		void compile(class GraphicsContext* const gfx_context, class Swapchain* const swapchain);
		void execute_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGPass* pass, void* command_buffer);
		void reset();

		void create_physical_pass_if_necessary(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGPassHandle pass);
		void create_physical_resource_if_necessary(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGResourceHandle pass);

	protected:
		ImageResourceFrameAllocator image_resource_allocator;
		BufferResourceFrameAllocator buffer_resource_allocator;
		RenderPassFrameAllocator render_pass_allocator;

		RenderGraphRegistry registry;

		std::vector<RGPassHandle> nonculled_passes;
	};
}
