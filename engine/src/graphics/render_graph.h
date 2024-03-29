#pragma once

#include <common.h>

#include <graphics/barrier_batcher.h>
#include <graphics/resource/image_types.h>
#include <graphics/resource/buffer_types.h>
#include <graphics/render_pass_types.h>
#include <memory/allocators/stack_allocator.h>

#include <unordered_set>

namespace Sunset
{
	using RGPassHandle = int32_t;
	using RGResourceHandle = int64_t;
	using RGResourceIndex = int32_t;
	using RGResourceVersion = int16_t;
	using RGResourceType = int16_t;

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
		RGPassHandle first_user;
		RGPassHandle last_user;
		std::vector<AccessFlags> access_flags;
		std::vector<ImageLayout> layouts;
		std::vector<RGPassHandle> producers;
		std::vector<RGPassHandle> consumers;
		bool b_is_persistent{ false };
		bool b_is_bindless{ false };
		uint32_t max_frame_lifetime{ 2 };
	};

	struct RGFrameData
	{
		RenderPassID current_pass{ 0 };
		class GraphicsContext* gfx_context{ nullptr };
		int32_t buffered_frame_number{ 0 };
		DescriptorSet* global_descriptor_set{ nullptr };
		DescriptorSet* pass_descriptor_set{ nullptr };
		PipelineStateID pass_pipeline_state{ 0 };
		ExecutionQueue* resource_deletion_queue{ nullptr };
		DescriptorBindlessResourceIndices pass_bindless_resources;
	};

	struct RGShaderDataSetup
	{
		// This is auxiliary and only used for passes that only need to bind a single pipeline state to run. Graph passes that don't specify
		// pipeline shaders will have to handle pipeline state creation and/or binding internally within the pass callback.
		std::vector<std::pair<PipelineShaderStageType, const char*>> pipeline_shaders;
		// Same note as above, this is entirely optional data within a graph pass definition
		std::optional<PushConstantPipelineData> push_constant_data;
		std::optional<PipelineRasterizerState> rasterizer_state;
		std::optional<PipelineAttachmentBlendState> attachment_blend;
		std::optional<PipelinePrimitiveTopologyType> primitive_topology_type;
		std::optional<Viewport> viewport;
		std::optional<bool> b_depth_write_enabled;
		std::optional<CompareOperation> depth_stencil_compare_op;
	};

	struct RGPassParameters
	{
		// See above, used to describe shader and pipeline setup for a given pass
		RGShaderDataSetup shader_setup;
		// Our input resources/attachments
		std::vector<RGResourceHandle> inputs;
		// Outputs that this pass either writes to or produces
		std::vector<RGResourceHandle> outputs;
		// Array layers of corresponding output entries in the outputs vector. (Only corresponds to image outputs. Default value is 0 for each output)
		std::vector<uint32_t> output_views;
		// Input resources that should be bound normally (auto computed)
		std::vector<RGResourceHandle> pass_inputs;
		// Input resources that are bindless and need special handling (auto computed)
		std::vector<RGResourceHandle> bindless_inputs;
		// Whether or not to skip automatic pass descriptor setup for this pass (global descriptor setup will still run)
		bool b_skip_auto_descriptor_setup{ false };
		// Whether or not to split mips from input images into separate descriptor writes (one per mip level). If false, uploads a single image with all the mip levels to GPU.
		bool b_split_input_image_mips{ false };
		// Whether or not prevent this pass from getting culled during render graph compilation. 
		bool b_force_keep_pass{ false };
	};

	struct RGPassCache
	{	
		DescriptorSet* global_descriptor_set[MAX_BUFFERED_FRAMES];
		phmap::flat_hash_map<Identity, DescriptorDataList> descriptors[MAX_BUFFERED_FRAMES];
		phmap::flat_hash_map<Identity, PipelineStateID> pipeline_states[MAX_BUFFERED_FRAMES];
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
		phmap::flat_hash_map<RGResourceHandle, RGResourceMetadata> resource_metadata;
		BarrierBatcher barrier_batcher;
		std::vector<BindingTableHandle> all_bindless_resource_handles;
		ExecutionQueue resource_deletion_queue;
		bool b_global_set_bound{ false };
	};

	class RenderGraph
	{
	public:
		RenderGraph() = default;
		~RenderGraph() = default;

		void initialize(class GraphicsContext* const gfx_context);
		void destroy(class GraphicsContext* const gfx_context);

		void begin(class GraphicsContext* const gfx_context);

		RGResourceHandle create_image(
			class GraphicsContext* const gfx_context,
			const AttachmentConfig& config,
			int32_t buffered_frame_number);

		RGResourceHandle register_image(
			class GraphicsContext* const gfx_context,
			ImageID image,
			int32_t buffered_frame_number);

		RGResourceHandle create_buffer(
			class GraphicsContext* const gfx_context,
			const BufferConfig& config,
			int32_t buffered_frame_number);

		RGResourceHandle register_buffer(
			class GraphicsContext* const gfx_context,
			BufferID buffer,
			int32_t buffered_frame_number);

		RGPassHandle add_pass(
			class GraphicsContext* const gfx_context,
			Identity name,
			RenderPassFlags pass_type,
			int32_t buffered_frame_number,
			std::function<void(RenderGraph&, RGFrameData&, void*)> execution_callback);

		RGPassHandle add_pass(
			class GraphicsContext* const gfx_context,
			Identity name,
			RenderPassFlags pass_type,
			int32_t buffered_frame_number,
			const RGPassParameters& params,
			std::function<void(RenderGraph&, RGFrameData&, void*)> execution_callback);

		void add_pass_resource_barrier(
			RGResourceHandle resource,
			RGPassHandle pass,
			AccessFlags dst_access,
			ImageLayout dst_layout = ImageLayout::Undefined,
			int32_t buffered_frame_number = 0);

		void submit(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, int32_t buffered_frame_number, bool b_offline = false);

		void queue_global_descriptor_writes(class GraphicsContext* const gfx_context, uint32_t buffered_frame, const std::initializer_list<DescriptorBufferDesc>& buffers);

		size_t get_physical_resource(RGResourceHandle resource, int32_t buffered_frame_number);

	protected:
		void update_reference_counts(RGPass* pass, RenderGraphRegistry& registry);
		void update_resource_param_producers_and_consumers(RGPass* pass, RenderGraphRegistry& registry);
		void update_present_pass_status(RGPass* pass, RenderGraphRegistry& registry);
		void cull_graph_passes(class GraphicsContext* const gfx_context, int32_t buffered_frame_number);
		void compute_resource_first_and_last_users(class GraphicsContext* const gfx_context, int32_t buffered_frame_number);
		void compute_resource_barriers(class GraphicsContext* const gfx_context, int32_t buffered_frame_number);
		void compile(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, int32_t buffered_frame_number);

		void execute_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGPass* pass, RGFrameData& frame_data, void* command_buffer);

		void setup_physical_pass_and_resources(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGPass* pass, RGFrameData& frame_data, void* command_buffer);
		void setup_physical_resource(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGPass* pass, RGResourceHandle resource, RGFrameData& frame_data, bool b_is_graphics_pass = true, bool b_is_input_resource = false);
		void tie_resource_to_pass_config_attachments(class GraphicsContext* const gfx_context, RGResourceHandle resource, RGPass* pass, int32_t buffered_frame_number, uint32_t resource_params_index, bool b_is_input_resource = false);
		void setup_pass_input_resource_bindless_type(class GraphicsContext* const gfx_context, RGResourceHandle resource, RGPass* pass, int32_t buffered_frame_number);
		void setup_pass_pipeline_state(class GraphicsContext* const gfx_context, RGPass* pass, RGFrameData& frame_data, void* command_buffer);
		void setup_pass_descriptors(class GraphicsContext* const gfx_context, RGPass* pass, RGFrameData& frame_data, void* command_buffer);
		void bind_pass_descriptors(class GraphicsContext* const gfx_context, RGPass* pass, RGFrameData& frame_data, void* command_buffer);
		void push_pass_constants(class GraphicsContext* const gfx_context, RGPass* pass, void* command_buffer);
		void update_transient_resources(class GraphicsContext* const gfx_context, RGPass* pass, int32_t buffered_frame_number);
		void free_physical_resources(class GraphicsContext* const gfx_context, int32_t current_buffered_frame);

		void reset(class GraphicsContext* const gfx_context, int32_t current_buffered_frame);

	protected:
		RGPassCache pass_cache;

		ImageResourceFrameAllocator image_resource_allocator[MAX_BUFFERED_FRAMES];
		BufferResourceFrameAllocator buffer_resource_allocator[MAX_BUFFERED_FRAMES];
		RenderPassFrameAllocator render_pass_allocator[MAX_BUFFERED_FRAMES];

		RenderGraphRegistry registries[MAX_BUFFERED_FRAMES];

		std::vector<RGPassHandle> nonculled_passes[MAX_BUFFERED_FRAMES];

		std::vector<DescriptorBufferDesc> queued_buffer_global_writes[MAX_BUFFERED_FRAMES];
	};
}
