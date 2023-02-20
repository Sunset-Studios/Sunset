#pragma once

#include <common.h>

#include <graphics/resource/image_types.h>
#include <graphics/resource/buffer_types.h>
#include <graphics/render_pass_types.h>
#include <memory/allocators/stack_allocator.h>

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
		return ((RGResourceHandle)index << 32) | ((RGResourceHandle)type << 16) | version;
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
		size_t physical_id{ 0 };
		std::vector<RGPassHandle> producers;
		std::vector<RGPassHandle> consumers;
		RGPassHandle first_user;
		RGPassHandle last_user;
		bool b_is_external{ false };
	};

	struct RGFrameData
	{
		RenderPassID current_pass{ 0 };
		class GraphicsContext* gfx_context{ nullptr };
	};

	struct RGShaderDescriptorDeclaration
	{
		uint32_t count{ 0 };
		DescriptorType type;
		PipelineShaderStageType shader_stages;
		bool b_supports_bindless{ false };
	};

	struct RGPipelineShaders
	{
		
	};

	struct RGShaderDataSetup
	{
		// For now the description declarations are needed to know how to create the per-pass descriptor layouts and sets. Ideally we won't need
		// this in the future after we make descriptors entirely dependent on spirv-reflect generated layouts/sets.
		std::vector<RGShaderDescriptorDeclaration> declarations;
		// This is auxiliary and only used for passes that only need to bind a single pipeline state to run. Graph passes that don't specify
		// pipeline shaders will have to handle pipeline state creation and/or binding internally within the pass callback.
		std::vector<std::pair<PipelineShaderStageType, const char*>> pipeline_shaders;
		// Same note as above, this is entirely optional data within a graph pass definition
		PushConstantPipelineData push_constant_data;
	};

	struct RGPassParameters
	{
		// See above, used to describe shader and pipeline setup for a given pass
		RGShaderDataSetup shader_setup;
		// Our input resources/attachments
		std::vector<RGResourceHandle> inputs;
		// Outputs that this pass either writes to or produces
		std::vector<RGResourceHandle> outputs;
	};

	class RGPass
	{
		friend class RenderGraph;
	protected:
		RGPassHandle handle;
		RenderPassConfig pass_config;
		RGPassParameters parameters;
		std::function<void(class RenderGraph&, RGFrameData&, void*)> executor;
		size_t physical_id{ 0 };
		size_t pipeline_state_id{ 0 };
		int32_t reference_count{ 0 };
	};

	using ImageResourceFrameAllocator = StaticFrameAllocator<RGImageResource, 256>;
	using BufferResourceFrameAllocator = StaticFrameAllocator<RGBufferResource, 256>;
	using RenderPassFrameAllocator = StaticFrameAllocator<RGPass, 128>;

	// All render graph registry resources (aside from render passes) should be transient and therefore do not need serious caching.
	// For this reason the pass cache is the only thing we don't clear out per-frame. Any resource that need to survive multiple frames
	// should be allocated externally and registered to the render graph as external resources.
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
		std::unordered_map<Identity, DescriptorData[MAX_BUFFERED_FRAMES]> pass_descriptor_cache;
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

		RGResourceHandle register_image(
			class GraphicsContext* const gfx_context,
			ImageID image);

		RGResourceHandle create_buffer(
			class GraphicsContext* const gfx_context,
			const BufferConfig& config);

		RGResourceHandle register_buffer(
			class GraphicsContext* const gfx_context,
			BufferID buffer);

		RGPassHandle add_pass(
			class GraphicsContext* const gfx_context,
			Identity name,
			RenderPassFlags pass_type,
			std::function<void(RenderGraph&, RGFrameData&, void*)> execution_callback);

		RGPassHandle add_pass(
			class GraphicsContext* const gfx_context,
			Identity name,
			RenderPassFlags pass_type,
			const RGPassParameters& params,
			std::function<void(RenderGraph&, RGFrameData&, void*)> execution_callback);

		void submit(class GraphicsContext* const gfx_context, class Swapchain* const swapchain);

		void inject_global_descriptors(class GraphicsContext* const gfx_context, uint16_t buffered_frame_index, const std::initializer_list<DescriptorBuildData>& descriptor_build_datas);

		size_t get_physical_resource(RGResourceHandle resource);

	protected:
		void update_reference_counts(RGPass* pass);
		void update_resource_param_producers_and_consumers(RGPass* pass);
		void cull_graph_passes(class GraphicsContext* const gfx_context);
		void compute_resource_first_and_last_users(class GraphicsContext* const gfx_context);
		void create_dummy_pipeline_layout(class GraphicsContext* const gfx_context);
		void compile(class GraphicsContext* const gfx_context, class Swapchain* const swapchain);

		void execute_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGPass* pass, RGFrameData& frame_data, void* command_buffer);

		void setup_physical_pass_and_resources(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGPassHandle pass, void* command_buffer);
		void setup_physical_resource(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGResourceHandle pass);
		void tie_resource_to_pass_config_attachments(class GraphicsContext* const gfx_context, RGResourceHandle resource, RGPass* pass);
		void setup_pass_pipeline_state_if_necessary(class GraphicsContext* const gfx_context, RGPass* pass, void* command_buffer);
		void setup_pass_descriptors(class GraphicsContext* const gfx_context, RGPass* pass, void* command_buffer);
		void bind_global_descriptors(class GraphicsContext* const gfx_context, void* command_buffer);
		void bind_pass_descriptors(class GraphicsContext* const gfx_context, RGPass* pass, void* command_buffer);
		void update_transient_resources(class GraphicsContext* const gfx_context, RGPass* pass);

		void reset();

	protected:
		ImageResourceFrameAllocator image_resource_allocator;
		BufferResourceFrameAllocator buffer_resource_allocator;
		RenderPassFrameAllocator render_pass_allocator;

		RenderGraphRegistry registry;

		std::vector<RGPassHandle> nonculled_passes;

		DescriptorData global_descriptor_datas[MAX_BUFFERED_FRAMES];
		ShaderLayoutID dummy_pipeline_layout{ 0 };
	};
}
