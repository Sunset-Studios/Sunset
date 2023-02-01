#include <graphics/api/vulkan/vk_descriptor.h>
#include <graphics/api/vulkan/vk_context.h>
#include <graphics/graphics_context.h>
#include <graphics/descriptor.h>
#include <graphics/asset_pool.h>
#include <graphics/pipeline_state.h>
#include <graphics/resource/shader_pipeline_layout.h>

namespace Sunset
{
	void VulkanDescriptorLayout::build(GraphicsContext* const gfx_context, const std::vector<DescriptorBinding>& bindings)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		assert(context_state != nullptr);

		std::vector<VkDescriptorSetLayoutBinding> vk_bindings{ VK_FROM_SUNSET_DESCRIPTOR_BINDINGS(bindings) };

		VkDescriptorSetLayoutCreateInfo set_create_info = {};
		set_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		set_create_info.pNext = nullptr;
		set_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
		set_create_info.flags = 0;
		set_create_info.pBindings = vk_bindings.data();
		vkCreateDescriptorSetLayout(context_state->get_device(), &set_create_info, nullptr, &layout);
	}

	void VulkanDescriptorLayout::destroy(class GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		assert(context_state != nullptr);

		vkDestroyDescriptorSetLayout(context_state->get_device(), layout, nullptr);
	}

	bool VulkanDescriptorSet::build(GraphicsContext* const gfx_context, DescriptorLayout* descriptor_layout, void* descriptor_pool)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		assert(context_state != nullptr);

		VkDescriptorSetLayout set_layout = static_cast<VkDescriptorSetLayout>(descriptor_layout->get());

		VkDescriptorSetAllocateInfo set_alloc_info = {};
		set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		set_alloc_info.pNext = nullptr;
		set_alloc_info.pSetLayouts = &set_layout;
		set_alloc_info.descriptorPool = static_cast<VkDescriptorPool>(descriptor_pool);
		set_alloc_info.descriptorSetCount = 1;

		VkResult alloc_result = vkAllocateDescriptorSets(context_state->get_device(), &set_alloc_info, &descriptor_set);
		switch (alloc_result)
		{
		case VK_SUCCESS:
			return true;
		default:
			return false;
		}
	}

	void VulkanDescriptorSet::bind(class GraphicsContext* const gfx_context, void* cmd_buffer, PipelineStateID pipeline_state, const std::vector<uint32_t>& dynamic_buffer_offsets, uint32_t set_index)
	{
		assert(pipeline_state != 0);

		ShaderPipelineLayout* const pipeline_layout = CACHE_FETCH(PipelineState, pipeline_state)->get_state_data().layout;
		VkPipelineLayout layout = static_cast<VkPipelineLayout>(pipeline_layout->get_data());
		VkCommandBuffer buffer = static_cast<VkCommandBuffer>(cmd_buffer);

		assert(layout != nullptr);
		assert(buffer != nullptr);

		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, set_index, 1, &descriptor_set, static_cast<uint32_t>(dynamic_buffer_offsets.size()), dynamic_buffer_offsets.data());
	}

	void VulkanDescriptorSetAllocator::configure_pool_sizes(const std::initializer_list<std::pair<DescriptorType, uint32_t>>& sizes)
	{
		pool_sizes.clear();
		pool_sizes.reserve(sizes.size());
		for (const auto& pool_size : sizes)
		{
			pool_sizes.push_back({ VK_FROM_SUNSET_DESCRIPTOR_TYPE(pool_size.first), pool_size.second });
		}
	}

	DescriptorSet* VulkanDescriptorSetAllocator::allocate(GraphicsContext* const gfx_context, DescriptorLayout* descriptor_layout)
	{
		if (current_pool == nullptr)
		{
			current_pool = get_new_pool(gfx_context);
			used_pools.push_back(current_pool);
		}

		DescriptorSet* new_descriptor_set = GlobalAssetPools<DescriptorSet>::get()->allocate();
		if (!new_descriptor_set->build(gfx_context, descriptor_layout, current_pool))
		{
			// Try again with a new pool as a failure to allocate the set indicates a potential OOM on the descriptor pool
			current_pool = get_new_pool(gfx_context);
			used_pools.push_back(current_pool);
			if (!new_descriptor_set->build(gfx_context, descriptor_layout, current_pool))
			{
				GlobalAssetPools<DescriptorSet>::get()->deallocate(new_descriptor_set);
				return nullptr;
			}
		}

		return new_descriptor_set;
	}

	void VulkanDescriptorSetAllocator::destroy(void* device)
	{
		VkDevice vk_device = static_cast<VkDevice>(device);
		assert(vk_device != nullptr);

		for (const auto& pool : free_pools)
		{
			vkDestroyDescriptorPool(vk_device, pool, nullptr);
		}

		for (auto pool : used_pools)
		{
			vkDestroyDescriptorPool(vk_device, pool, nullptr);
		}
	}

	void VulkanDescriptorSetAllocator::reset(GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		assert(context_state != nullptr);

		for (const auto& pool : used_pools) {
			vkResetDescriptorPool(context_state->get_device(), pool, 0);
			free_pools.push_back(pool);
		}

		used_pools.clear();
		current_pool = nullptr;
	}

	VkDescriptorPool VulkanDescriptorSetAllocator::get_new_pool(class GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		assert(context_state != nullptr);

		VkDescriptorPool pool{ nullptr };

		if (free_pools.size() > 0)
		{
			pool = free_pools.back();
			free_pools.pop_back();
		}
		else
		{
			VkDescriptorPoolCreateInfo pool_create_info = {};
			pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_create_info.pNext = nullptr;
			pool_create_info.flags = 0;
			pool_create_info.maxSets = max_sets_per_pool;
			pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
			pool_create_info.pPoolSizes = pool_sizes.data();

			vkCreateDescriptorPool(context_state->get_device(), &pool_create_info, nullptr, &pool);
		}

		return pool;
	}
}
