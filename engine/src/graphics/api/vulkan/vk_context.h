#pragma once

#include <vk_types.h>
#include <vk_initializers.h>
#include <vk_sync.h>

#include <VkBootstrap.h>

namespace Sunset
{
	struct VulkanContextState
	{
		public:
			VkInstance instance{ nullptr };
			VkDebugUtilsMessengerEXT debug_messenger{ nullptr };
			VkSurfaceKHR surface{ nullptr };
			class Window* window{ nullptr };
			vkb::Device device;
			vkb::PhysicalDevice physical_device;
			VulkanSyncPool sync_pool;
			VulkanSemaphoreHandle present_semaphore{ -1 };
			VulkanSemaphoreHandle render_semaphore{ -1 };
			VulkanFenceHandle render_fence{ -1 };
			uint32_t frame_number{ 0 };

		public:
			VulkanContextState() = default;
			VulkanContextState(VulkanContextState&& other) = delete;
			VulkanContextState(const VulkanContextState& other) = delete;
			VulkanContextState& operator=(const VulkanContextState& other) = delete;

			VkDevice get_device()
			{
				return device.device;
			}

			VkPhysicalDevice get_gpu()
			{
				return physical_device.physical_device;
			}
	};

	class VulkanContext
	{
		public:
			VulkanContext() = default;
			~VulkanContext() = default;

		public:
			void initialize(class Window* const window);
			void destroy();
			void wait_for_gpu();

			void* get_state()
			{
				return &state;
			}

			uint32_t get_frame_number() const
			{
				return state.frame_number;
			}

			void advance_frame()
			{
				++state.frame_number;
			}

		public:
			VulkanContextState state;
	};

	void create_surface(VulkanContext* const vulkan_context, class Window* const window);
}