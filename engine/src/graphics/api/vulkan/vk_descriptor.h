#pragma once

#include <vk_types.h>
#include <vk_sync.h>

#include <graphics/descriptor_types.h>

namespace Sunset
{
	class VulkanDescriptorLayout
	{
	public:
		VulkanDescriptorLayout() = default;

		void build(class GraphicsContext* const gfx_context, const std::vector<DescriptorBinding>& bindings);
		void destroy(class GraphicsContext* const gfx_context);

		void* get() const
		{
			return layout;
		}

		bool supports_bindless() const
		{
			return b_supports_bindless;
		}

	protected:
		VkDescriptorSetLayout layout;
		bool b_supports_bindless{ false };
	};

	class VulkanDescriptorSet
	{
	public:
		VulkanDescriptorSet() = default;

		bool build(class GraphicsContext* const gfx_context, class DescriptorLayout* descriptor_layout, void* descriptor_pool);
		void bind(class GraphicsContext* const gfx_context, void* cmd_buffer, ShaderLayoutID layout, PipelineStateType pipeline_state_type = PipelineStateType::Graphics, const std::vector<uint32_t>& dynamic_buffer_offsets = {}, uint32_t set_index = 0);
		void* get() const
		{
			return descriptor_set;
		}

	protected:
		VkDescriptorSet descriptor_set;
	};

	class VulkanDescriptorSetAllocator
	{
	public:
		VulkanDescriptorSetAllocator() = default;
		
		void configure_pool_sizes(const std::initializer_list<std::pair<DescriptorType, uint32_t>>& sizes);
		void configure_max_sets_per_pool(uint32_t max_sets)
		{
			max_sets_per_pool = max_sets;
		}

		class DescriptorSet* allocate(class GraphicsContext* const gfx_context, class DescriptorLayout* descriptor_layout);
		void destroy(void* device);
		void reset(class GraphicsContext* const gfx_context);

	private:
		VkDescriptorPool get_new_pool(class GraphicsContext* const gfx_context);

	private:
		uint32_t max_sets_per_pool{ 256 };
		VkDescriptorPool current_pool{ nullptr };
		std::vector<VkDescriptorPoolSize> pool_sizes;
		std::vector<VkDescriptorPool> used_pools;
		std::vector<VkDescriptorPool> free_pools;
	};
}
