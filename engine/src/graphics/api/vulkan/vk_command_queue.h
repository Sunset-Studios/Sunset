#pragma once

#include <vk_types.h>
#include <vk_initializers.h>

namespace Sunset
{
	struct VulkanFrameCommandPoolData
	{
		VkCommandPool command_pool;
		VkCommandBuffer command_buffer;
	};

	struct VulkanCommandQueueData
	{
		VkQueue graphics_queue;
		uint32_t graphics_queue_family;
		VulkanFrameCommandPoolData frame_command_pool_data[MAX_BUFFERED_FRAMES];
	};

	class VulkanCommandQueue
	{
	public:
		VulkanCommandQueue() = default;
		~VulkanCommandQueue() = default;

	public:
		void initialize(class GraphicsContext* const gfx_context);
		void destroy(class GraphicsContext* const gfx_context);

		void* get_data()
		{
			return &data;
		}

		void new_command_pool(class GraphicsContext* const gfx_context, void* command_pool_ptr, uint16_t buffered_frame_number = 0);
		void new_command_buffers(class GraphicsContext* const gfx_context, void* command_buffer_ptr, uint16_t count = 1, uint16_t buffered_frame_number = 0);
		void* begin_one_time_buffer_record(class GraphicsContext* const gfx_context);
		void end_one_time_buffer_record(class GraphicsContext* const gfx_context);
		void submit(class GraphicsContext* const gfx_context);

	protected:
		VulkanCommandQueueData data;
	};
}
