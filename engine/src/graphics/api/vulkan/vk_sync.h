#pragma once

#include <vector>
#include <cassert>

#include <vk_types.h>
#include <vk_initializers.h>

namespace Sunset
{
	using VulkanSemaphoreHandle = int16_t;
	using VulkanFenceHandle = int16_t;

	struct VulkanSyncPool
	{
		public:
			VulkanSyncPool() = default;
			VulkanSyncPool(VulkanSyncPool&& other) = delete;
			VulkanSyncPool(const VulkanSyncPool& other) = delete;
			VulkanSyncPool& operator=(const VulkanSyncPool& other) = delete;

			VulkanFenceHandle new_fence(struct VulkanContextState* const context_state);
			VulkanSemaphoreHandle new_semaphore(struct VulkanContextState* const context_state);

			const VkFence& get_fence(VulkanFenceHandle fence_idx) const
			{
				assert(fence_idx >= 0 && fence_idx < fences.size());
				return fences[fence_idx];
			}

			const VkSemaphore get_semaphore(VulkanSemaphoreHandle semaphore_idx) const
			{
				assert(semaphore_idx >= 0 && semaphore_idx < semaphores.size());
				return semaphores[semaphore_idx];
			}

			void release_fence(struct VulkanContextState* const context_state, VulkanFenceHandle fence_idx);
			void release_semaphore(struct VulkanContextState* const context_state, VulkanSemaphoreHandle semaphore_idx);

		public:
			std::vector<VkSemaphore> semaphores;
			std::vector<VkFence> fences;
	};
}
