#include <graphics/api/vulkan/vk_sync.h>
#include <graphics/graphics_context.h>

namespace Sunset
{
	VulkanFenceHandle VulkanSyncPool::new_fence(VulkanContextState* const context_state)
	{
		const VulkanFenceHandle fence_idx = static_cast<VulkanFenceHandle>(fences.size());
		fences.push_back(VkFence());

		VkFenceCreateInfo fence_create_info = {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.pNext = nullptr;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VK_CHECK(vkCreateFence(context_state->get_device(), &fence_create_info, nullptr, &fences[fence_idx]));

		return fence_idx;
	}

	VulkanSemaphoreHandle VulkanSyncPool::new_semaphore(VulkanContextState* const context_state)
	{
		const VulkanSemaphoreHandle semaphore_idx = static_cast<VulkanSemaphoreHandle>(semaphores.size());
		semaphores.push_back(VkSemaphore());

		VkSemaphoreCreateInfo semaphore_create_info = {};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore_create_info.pNext = nullptr;
		semaphore_create_info.flags = 0;

		VK_CHECK(vkCreateSemaphore(context_state->get_device(), &semaphore_create_info, nullptr, &semaphores[semaphore_idx]));

		return semaphore_idx;
	}

	void VulkanSyncPool::release_fence(VulkanContextState* const context_state, VulkanFenceHandle fence_idx)
	{
		assert(fence_idx >= 0 && fence_idx < fences.size());
		
		VkFence fence = fences[fence_idx];
		fences.erase(fences.begin() + fence_idx);

		vkDestroyFence(context_state->get_device(), fence, nullptr);
	}

	void VulkanSyncPool::release_semaphore(VulkanContextState* const context_state, VulkanSemaphoreHandle semaphore_idx)
	{
		assert(semaphore_idx >= 0 && semaphore_idx < semaphores.size());

		VkSemaphore semaphore = semaphores[semaphore_idx];
		semaphores.erase(semaphores.begin() + semaphore_idx);

		vkDestroySemaphore(context_state->get_device(), semaphore, nullptr);
	}
}
