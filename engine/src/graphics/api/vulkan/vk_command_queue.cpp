#include <graphics/api/vulkan/vk_command_queue.h>
#include <graphics/graphics_context.h>

#include <VkBootstrap.h>

namespace Sunset
{
	void VulkanCommandQueue::initialize(GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		// TODO: May want a way to specify what kind of queue we need (i.e. transfer queues, present queues, compute queues, etc.)
		data.graphics_queue = context_state->device.get_queue(vkb::QueueType::graphics).value();
		data.graphics_queue_family = context_state->device.get_queue_index(vkb::QueueType::graphics).value();

		new_command_pool(gfx_context, &data.command_pool);
		new_command_buffers(gfx_context, &data.command_buffer);
	}

	void VulkanCommandQueue::destroy(GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		vkDestroyCommandPool(context_state->get_device(), data.command_pool, nullptr);
	}

	void VulkanCommandQueue::new_command_pool(GraphicsContext* const gfx_context, void* command_pool_ptr)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		VkCommandPoolCreateInfo command_pool_info = {};
		command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_info.pNext = nullptr;

		command_pool_info.queueFamilyIndex = data.graphics_queue_family;
		command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_CHECK(vkCreateCommandPool(context_state->get_device(), &command_pool_info, nullptr, static_cast<VkCommandPool*>(command_pool_ptr)));
	}

	void VulkanCommandQueue::new_command_buffers(class GraphicsContext* const gfx_context, void* command_buffer_ptr, uint16_t count)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		VkCommandBufferAllocateInfo cmd_allocate_info = {};
		cmd_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_allocate_info.pNext = nullptr;

		cmd_allocate_info.commandPool = data.command_pool;
		cmd_allocate_info.commandBufferCount = count;
		cmd_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VK_CHECK(vkAllocateCommandBuffers(context_state->get_device(), &cmd_allocate_info, static_cast<VkCommandBuffer*>(command_buffer_ptr)));
	}

	void* VulkanCommandQueue::begin_one_time_buffer_record(class GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		VK_CHECK(vkResetCommandBuffer(data.command_buffer, 0));

		VkCommandBufferBeginInfo cmd_begin_info = {};
		cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_begin_info.pNext = nullptr;
		cmd_begin_info.pInheritanceInfo = nullptr;
		cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK(vkBeginCommandBuffer(data.command_buffer, &cmd_begin_info));

		return data.command_buffer;
	}

	void VulkanCommandQueue::end_one_time_buffer_record(class GraphicsContext* const gfx_context)
	{
		VK_CHECK(vkEndCommandBuffer(data.command_buffer));
	}


	void VulkanCommandQueue::submit(class GraphicsContext* const gfx_context)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = nullptr;

		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSemaphore present_semaphore = context_state->sync_pool.get_semaphore(context_state->present_semaphore);
		VkSemaphore render_semaphore = context_state->sync_pool.get_semaphore(context_state->render_semaphore);
		VkFence render_fence = context_state->sync_pool.get_fence(context_state->render_fence);

		submit_info.pWaitDstStageMask = &wait_stage;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &present_semaphore;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &render_semaphore;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &data.command_buffer;

		VK_CHECK(vkQueueSubmit(data.graphics_queue, 1, &submit_info, render_fence));
	}
}