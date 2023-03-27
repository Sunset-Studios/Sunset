#pragma once

#include <common.h>
#include <graphics/descriptor_types.h>
#include <graphics/resource/resource_cache.h>

namespace Sunset
{
	constexpr uint32_t MAX_DESCRIPTOR_BINDINGS = 16536;

	// Binding table is mostly used for bindless descriptor sets, so that we have a simple interface
	// for pooling array descriptors.
	struct DescriptorBindingTable
	{
		struct BindingTableEntry
		{
			uint32_t total_bindings_count;
			std::vector<int32_t> free_indices;
			std::vector<int32_t> bound_indices;
		};
		std::unordered_map<uint32_t, BindingTableEntry> binding_table;

		inline bool has_binding_slot(uint32_t slot);
		void add_binding_slot(uint32_t slot, uint32_t count);
		BindingTableHandle get_new(uint32_t slot);
		void free(BindingTableHandle handle);
		void reset(uint32_t slot);
	};

	// BEGIN - Descriptor Set Layout
	template<class Policy>
	class GenericDescriptorLayout
	{
	public:
		GenericDescriptorLayout() = default;

		void add_binding(const DescriptorBinding& binding)
		{
			bindings.push_back(binding);
		}

		void set_bindings(const std::vector<DescriptorBinding>& new_bindings)
		{
			bindings = new_bindings;
		}

		std::vector<DescriptorBinding>& get_bindings()
		{
			return bindings;
		}

		void build(class GraphicsContext* const gfx_context)
		{
			descriptor_layout_policy.build(gfx_context, bindings);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			descriptor_layout_policy.destroy(gfx_context);
		}

		void* get() const
		{
			return descriptor_layout_policy.get();
		}

		bool supports_bindless() const
		{
			return descriptor_layout_policy.supports_bindless();
		}

	private:
		Policy descriptor_layout_policy;
		std::vector<DescriptorBinding> bindings;
	};

	class NoopDescriptorLayout
	{
	public:
		NoopDescriptorLayout() = default;

		void build(class GraphicsContext* const gfx_context, const std::vector<DescriptorBinding>& bindings)
		{ }

		void destroy(class GraphicsContext* const gfx_context)
		{ }

		void* get() const
		{
			return nullptr;
		}

		bool supports_bindless() const
		{
			return false;
		}
	};

#if USE_VULKAN_GRAPHICS
	class DescriptorLayout : public GenericDescriptorLayout<VulkanDescriptorLayout>
	{ };
#else
	class DescriptorLayout : public GenericDescriptorLayout<NoopDescriptorLayout>
	{ };
#endif

	class DescriptorLayoutFactory
	{
	public:
		static DescriptorLayoutID create(class GraphicsContext* const gfx_context, const std::vector<DescriptorBinding>& bindings);
	};

	DEFINE_RESOURCE_CACHE(DescriptorLayoutCache, DescriptorLayoutID, DescriptorLayout);
	// END - Descriptor Set Layout

	// BEGIN - Descriptor Set
	template<class Policy>
	class GenericDescriptorSet
	{
	public:
		GenericDescriptorSet() = default;

		bool build(class GraphicsContext* const gfx_context, class DescriptorLayout* descriptor_layout, void* descriptor_pool)
		{
			return descriptor_set_policy.build(gfx_context, descriptor_layout, descriptor_pool);
		}

		void bind(class GraphicsContext* const gfx_context, void* cmd_buffer, ShaderLayoutID layout, PipelineStateType pipeline_state_type = PipelineStateType::Graphics, uint32_t set_index = 0)
		{
			descriptor_set_policy.bind(gfx_context, cmd_buffer, layout, pipeline_state_type, dynamic_buffer_offsets, set_index);
		}

		void* get() const
		{
			return descriptor_set_policy.get();
		}

		BindingTableHandle get_free_bindless_index(uint32_t slot)
		{
			return binding_table.get_new(slot);
		}

		void release_bindless_index(BindingTableHandle handle)
		{
			binding_table.free(handle);
		}

		void register_bindless_slot(uint32_t slot, uint32_t count)
		{
			binding_table.add_binding_slot(slot, count);
		}

		void set_dynamic_buffer_offsets(const std::vector<uint32_t>& buffer_offsets)
		{
			dynamic_buffer_offsets = buffer_offsets;
		}

	private:
		Policy descriptor_set_policy;
		DescriptorBindingTable binding_table;
		std::vector<uint32_t> dynamic_buffer_offsets;
	};

	class NoopDescriptorSet
	{
	public:
		NoopDescriptorSet() = default;

		bool build(class GraphicsContext* const gfx_context, class DescriptorLayout* descriptor_layout, void* descriptor_pool)
		{ }

		void bind(class GraphicsContext* const gfx_context, void* cmd_buffer, ShaderLayoutID layout, const std::vector<uint32_t>& dynamic_buffer_offsets = {}, uint32_t set_index = 0)
		{ }

		void bind(class GraphicsContext* const gfx_context, void* cmd_buffer, ShaderLayoutID layout, PipelineStateType pipeline_state_type = PipelineStateType::Graphics, const std::vector<uint32_t>& dynamic_buffer_offsets = {}, uint32_t set_index = 0)
		{ }

		void* get() const
		{
			return nullptr;
		}
	};

#if USE_VULKAN_GRAPHICS
	class DescriptorSet : public GenericDescriptorSet<VulkanDescriptorSet>
	{ };
#else
	class DescriptorSet : public GenericDescriptorSet<NoopDescriptorSet>
	{ };
#endif
	// END - Descriptor Set

	// BEGIN - Descriptor Set Allocator
	template<class Policy>
	class GenericDescriptorSetAllocator
	{
	public:
		GenericDescriptorSetAllocator() = default;

		void configure_pool_sizes(const std::initializer_list<std::pair<DescriptorType, uint32_t>>& sizes)
		{
			allocator_policy.configure_pool_sizes(sizes);
		}

		void configure_max_sets_per_pool(uint32_t max_sets)
		{
			allocator_policy.configure_max_sets_per_pool(max_sets);
		}

		DescriptorSet* allocate(class GraphicsContext* const gfx_context, class DescriptorLayout* descriptor_layout)
		{
			return allocator_policy.allocate(gfx_context, descriptor_layout);
		}

		void destroy(void* device)
		{
			allocator_policy.destroy(device);
		}

		void reset(class GraphicsContext* const gfx_context)
		{
			allocator_policy.reset(gfx_context);
		}

	private:
		Policy allocator_policy;
	};

	class NoopDescriptorSetAllocator
	{
	public:
		NoopDescriptorSetAllocator() = default;

		void configure_pool_sizes(const std::initializer_list<std::pair<DescriptorType, uint32_t>>& sizes)
		{ }

		void configure_max_sets_per_pool(uint32_t max_sets)
		{ }

		DescriptorSet* allocate(class GraphicsContext* const gfx_context, class DescriptorLayout* descriptor_layout)
		{ }

		void destroy(void* device)
		{ }

		void reset(class GraphicsContext* const gfx_context)
		{ }
	};

#if USE_VULKAN_GRAPHICS
	class DescriptorSetAllocator : public GenericDescriptorSetAllocator<VulkanDescriptorSetAllocator>
	{ };
#else
	class DescriptorSetAllocator : public GenericDescriptorSetAllocator<NoopDescriptorSetAllocator>
	{ };
#endif

	class DescriptorSetAllocatorFactory
	{
	public:
		static DescriptorSetAllocator* create(class GraphicsContext* const gfx_context)
		{
			DescriptorSetAllocator* descriptor_set_alloc = new DescriptorSetAllocator;
			return descriptor_set_alloc;
		}
	};
	// END - Descriptor Set Allocator

	// BEGIN - Descriptor Set Builder
	class DescriptorSetBuilder
	{
	public:
		DescriptorSetBuilder() = default;

		static DescriptorSetBuilder begin(class GraphicsContext* const context);

		DescriptorSetBuilder& bind_buffer(uint16_t binding, class Buffer* buffer, size_t range_size, uint32_t count, DescriptorType type, PipelineShaderStageType shader_stages, size_t buffer_offset, bool b_supports_bindless = false);
		DescriptorSetBuilder& bind_buffer(const DescriptorBuildData& build_data);
		DescriptorSetBuilder& bind_image(uint16_t binding, class Image* image, size_t range_size, uint32_t count, DescriptorType type, PipelineShaderStageType shader_stages, bool b_supports_bindless = false);
		DescriptorSetBuilder& bind_image(const DescriptorBuildData& build_data);

		bool build(DescriptorSet*& out_descriptor_set, DescriptorLayoutID& out_descriptor_layout);

	private:
		class GraphicsContext* gfx_context{ nullptr };
		std::vector<DescriptorBinding> bindings;
		std::vector<DescriptorWrite> descriptor_writes;
	};
	// END - Descriptor Set Builder

	// BEGIN - Descriptor Helpers
	class DescriptorHelpers
	{
	public:
		static void inject_descriptors(class GraphicsContext* const gfx_context, DescriptorData& out_descriptor_data, const std::vector<DescriptorBuildData>& descriptor_build_datas);
		static DescriptorSet* new_descriptor_set_with_layout(class GraphicsContext* const gfx_context, DescriptorLayoutID descriptor_layout);
		static void write_descriptors(class GraphicsContext* const gfx_context, DescriptorSet* descriptor_set, std::vector<DescriptorWrite>& descriptor_writes);
		static void write_bindless_descriptors(class GraphicsContext* const gfx_context, const std::vector<DescriptorBindlessWrite>& descriptor_writes, BindingTableHandle* out_binding_table_handles);
		static void free_bindless_image_descriptors(class GraphicsContext* const gfx_context, DescriptorSet* descriptor_set, const std::vector<BindingTableHandle> indices);
		static std::vector<DescriptorBindlessWrite> new_descriptor_image_bindless_writes(class DescriptorSet* set, ImageID image, bool b_split_image_mips = true);
	};
	// END - Descriptor Helpers
}
