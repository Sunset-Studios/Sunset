#include <graphics/api/vulkan/vk_command_queue.h>
#include <graphics/graphics_context.h>

#include <VkBootstrap.h>

namespace Sunset
{
	void VulkanCommandQueue::initialize(void* gfx_context_state, DeviceQueueType queue_type)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context_state);

		switch (queue_type)
		{
		case Sunset::DeviceQueueType::Graphics:
			data.graphics_queue_family = context_state->device.get_queue_index(vkb::QueueType::graphics).value();
			break;
		case Sunset::DeviceQueueType::Compute:
			data.graphics_queue_family = context_state->device.get_queue_index(vkb::QueueType::compute).value();
			break;
		case Sunset::DeviceQueueType::Transfer:
			data.graphics_queue_family = context_state->device.get_queue_index(vkb::QueueType::transfer).value();
			break;
		default:
			data.graphics_queue_family = context_state->device.get_queue_index(vkb::QueueType::graphics).value();
			break;
		}

		switch (queue_type)
		{
		case Sunset::DeviceQueueType::Graphics:
			data.graphics_queue = context_state->device.get_queue(vkb::QueueType::graphics).value();
			break;
		case Sunset::DeviceQueueType::Compute:
			data.graphics_queue = context_state->device.get_queue(vkb::QueueType::compute).value();
			break;
		case Sunset::DeviceQueueType::Transfer:
			data.graphics_queue = context_state->device.get_queue(vkb::QueueType::transfer).value();
			break;
		default:
			data.graphics_queue = context_state->device.get_queue(vkb::QueueType::graphics).value();
			break;
		}

		for (int16_t frame_number = 0; frame_number < MAX_BUFFERED_FRAMES; ++frame_number)
		{
			new_command_pool(gfx_context_state, &data.frame_command_pool_data[frame_number].command_pool, frame_number);
			new_command_buffers(gfx_context_state, &data.frame_command_pool_data[frame_number].command_buffer, data.frame_command_pool_data[frame_number].command_pool, 1, frame_number);

			new_command_pool(gfx_context_state, &data.immediate_command_data[frame_number].command_pool);
			new_command_buffers(gfx_context_state, &data.immediate_command_data[frame_number].command_buffer, data.immediate_command_data[frame_number].command_pool);

			data.immediate_command_data[frame_number].fence = context_state->sync_pool.new_fence(context_state);

			VkFence render_fence = context_state->sync_pool.get_fence(data.immediate_command_data[frame_number].fence);
			vkResetFences(context_state->get_device(), 1, &render_fence);
		}
	}

	void VulkanCommandQueue::destroy(void* gfx_context_state)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context_state);

		for (int16_t frame_number = 0; frame_number < MAX_BUFFERED_FRAMES; ++frame_number)
		{
			vkDestroyCommandPool(context_state->get_device(), data.immediate_command_data[frame_number].command_pool, nullptr);

			VkFence render_fence = context_state->sync_pool.get_fence(data.immediate_command_data[frame_number].fence);
			vkDestroyFence(context_state->get_device(), render_fence, nullptr);

			vkDestroyCommandPool(context_state->get_device(), data.frame_command_pool_data[frame_number].command_pool, nullptr);
		}
	}

	void VulkanCommandQueue::new_command_pool(void* gfx_context_state, void* command_pool_ptr, uint16_t buffered_frame_number)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context_state);

		VkCommandPoolCreateInfo command_pool_info = {};
		command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_info.pNext = nullptr;

		command_pool_info.queueFamilyIndex = data.graphics_queue_family;
		command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_CHECK(vkCreateCommandPool(context_state->get_device(), &command_pool_info, nullptr, static_cast<VkCommandPool*>(command_pool_ptr)));
	}

	void VulkanCommandQueue::new_command_buffers(void* gfx_context_state, void* command_buffer_ptr, void* command_pool_ptr, uint16_t count, uint16_t buffered_frame_number)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context_state);
		VkCommandPool command_pool = static_cast<VkCommandPool>(command_pool_ptr);

		VkCommandBufferAllocateInfo cmd_allocate_info = {};
		cmd_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_allocate_info.pNext = nullptr;

		cmd_allocate_info.commandPool = command_pool;
		cmd_allocate_info.commandBufferCount = count;
		cmd_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VK_CHECK(vkAllocateCommandBuffers(context_state->get_device(), &cmd_allocate_info, static_cast<VkCommandBuffer*>(command_buffer_ptr)));
	}

	void* VulkanCommandQueue::begin_one_time_buffer_record(class GraphicsContext* const gfx_context, uint32_t buffered_frame_number)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		VkCommandBuffer& command_buffer = data.frame_command_pool_data[buffered_frame_number].command_buffer;

		VK_CHECK(vkResetCommandBuffer(command_buffer, 0));

		VkCommandBufferBeginInfo cmd_begin_info = {};
		cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_begin_info.pNext = nullptr;
		cmd_begin_info.pInheritanceInfo = nullptr;
		cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK(vkBeginCommandBuffer(command_buffer, &cmd_begin_info));

		return command_buffer;
	}

	void VulkanCommandQueue::end_one_time_buffer_record(class GraphicsContext* const gfx_context, uint32_t buffered_frame_number)
	{
		VkCommandBuffer& command_buffer = data.frame_command_pool_data[buffered_frame_number].command_buffer;
		VK_CHECK(vkEndCommandBuffer(command_buffer));
	}


	void VulkanCommandQueue::submit(class GraphicsContext* const gfx_context, uint32_t buffered_frame_number, bool b_offline)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());

		context_state->has_pending_work[buffered_frame_number].store(true, std::memory_order_release);

		VkCommandBuffer& command_buffer = data.frame_command_pool_data[buffered_frame_number].command_buffer;

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = nullptr;

		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSemaphore present_semaphore = context_state->sync_pool.get_semaphore(context_state->frame_sync_primitives[buffered_frame_number].present_semaphore);
		VkSemaphore render_semaphore = context_state->sync_pool.get_semaphore(context_state->frame_sync_primitives[buffered_frame_number].render_semaphore);
		VkFence render_fence = context_state->sync_pool.get_fence(context_state->frame_sync_primitives[buffered_frame_number].render_fence);

		submit_info.pWaitDstStageMask = &wait_stage;
		submit_info.waitSemaphoreCount = 1 - static_cast<uint32_t>(b_offline);
		submit_info.pWaitSemaphores = b_offline ? nullptr : &present_semaphore;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &render_semaphore;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		VK_CHECK(vkQueueSubmit(data.graphics_queue, 1, &submit_info, render_fence));
	}

	void VulkanCommandQueue::submit_immediate(class GraphicsContext* const gfx_context, int32_t buffered_frame_number, const std::function<void(void* cmd_buffer)>& buffer_update_fn)
	{
		VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
		VkCommandBuffer& command_buffer = data.immediate_command_data[buffered_frame_number].command_buffer;

		VkCommandBufferBeginInfo cmd_begin_info = {};
		cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_begin_info.pNext = nullptr;
		cmd_begin_info.pInheritanceInfo = nullptr;
		cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK(vkBeginCommandBuffer(command_buffer, &cmd_begin_info));

		buffer_update_fn(command_buffer);

		VK_CHECK(vkEndCommandBuffer(command_buffer));

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = nullptr;

		VkFence render_fence = context_state->sync_pool.get_fence(data.immediate_command_data[buffered_frame_number].fence);

		submit_info.pWaitDstStageMask = nullptr;
		submit_info.waitSemaphoreCount = 0;
		submit_info.pWaitSemaphores = nullptr;
		submit_info.signalSemaphoreCount = 0;
		submit_info.pSignalSemaphores = nullptr;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		VK_CHECK(vkQueueSubmit(data.graphics_queue, 1, &submit_info, render_fence));

		vkWaitForFences(context_state->get_device(), 1, &render_fence, true, 9999999999);
		vkResetFences(context_state->get_device(), 1, &render_fence);

		vkResetCommandPool(context_state->get_device(), data.immediate_command_data[buffered_frame_number].command_pool, 0);
	}
}
