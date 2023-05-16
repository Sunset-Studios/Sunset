#pragma once

#include <command_queue_types.h>

#include <vk_types.h>
#include <vk_sync.h>

namespace Sunset
{
	struct VulkanFrameCommandPoolData
	{
		VkCommandPool command_pool;
		VkCommandBuffer command_buffer;
	};

	struct VulkanImmediateCommandData
	{
		VkCommandPool command_pool;
		VkCommandBuffer command_buffer;
		VulkanFenceHandle fence;
	};

	struct VulkanCommandQueueData
	{
		VkQueue graphics_queue;
		uint32_t graphics_queue_family;
		VulkanFrameCommandPoolData frame_command_pool_data[MAX_BUFFERED_FRAMES];
		VulkanImmediateCommandData immediate_command_data;
	};

	class VulkanCommandQueue
	{
	public:
		VulkanCommandQueue() = default;
		~VulkanCommandQueue() = default;

	public:
		void initialize(void* gfx_context_state, DeviceQueueType queue_type);
		void destroy(void* gfx_context_state);

		void* get_data()
		{
			return &data;
		}

		void new_command_pool(void* gfx_context_state, void* command_pool_ptr, uint16_t buffered_frame_number = 0);
		void new_command_buffers(void* gfx_context_state, void* command_buffer_ptr, void* command_pool_ptr, uint16_t count = 1, uint16_t buffered_frame_number = 0);
		void* begin_one_time_buffer_record(class GraphicsContext* const gfx_context);
		void end_one_time_buffer_record(class GraphicsContext* const gfx_context);
		void submit(class GraphicsContext* const gfx_context, bool b_offline = false);
		void submit_immediate(class GraphicsContext* const gfx_context, const std::function<void(void* cmd_buffer)>& buffer_update_fn);

	protected:
		VulkanCommandQueueData data;
	};
}
