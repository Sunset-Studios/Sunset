#pragma once

#include <vk_types.h>
#include <vk_initializers.h>

namespace Sunset
{
	struct VulkanCommandQueueData
	{
		VkQueue graphics_queue;
		uint32_t graphics_queue_family;
		// TODO: Add support for multiple command queues and buffers
		VkCommandPool command_pool;
		VkCommandBuffer command_buffer;
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

		void new_command_pool(class GraphicsContext* const gfx_context, void* command_pool_ptr);
		void new_command_buffers(class GraphicsContext* const gfx_context, void* command_buffer_ptr, uint16_t count = 1);
		void* begin_one_time_buffer_record(class GraphicsContext* const gfx_context);
		void end_one_time_buffer_record(class GraphicsContext* const gfx_context);
		void submit(class GraphicsContext* const gfx_context);

	protected:
		VulkanCommandQueueData data;
	};
}
