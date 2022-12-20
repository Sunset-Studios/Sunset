#include <graphics/api/vulkan/vk_context.h>
#include <graphics/resource/buffer.h>
#include <graphics/pipeline_state.h>
#include <graphics/descriptor.h>
#include <core/common.h>
#include <window/window.h>

#include <SDL_vulkan.h>

namespace Sunset
{
	void VulkanContext::initialize(Window* const window)
	{
		vkb::InstanceBuilder builder;

		vkb::Instance instance_result = builder.set_app_name(ENGINE_NAME)
			.request_validation_layers(true)
			.require_api_version(1, 1, 0)
			.use_default_debug_messenger()
			.build()
			.value();

		state.instance = instance_result.instance;
		state.debug_messenger = instance_result.debug_messenger;

		create_surface(this, window);

		vkb::PhysicalDeviceSelector device_selector{ instance_result };
		state.physical_device = device_selector
			.set_minimum_version(1, 1)
			.set_surface(state.surface)
			.select()
			.value();

		vkb::DeviceBuilder device_builder{ state.physical_device };

		state.device = device_builder.build().value();
		state.window = window;

		for (int16_t frame_number = 0; frame_number < MAX_BUFFERED_FRAMES; ++frame_number)
		{
			state.frame_sync_primitives[frame_number].render_semaphore = state.sync_pool.new_semaphore(&state);
			state.frame_sync_primitives[frame_number].present_semaphore = state.sync_pool.new_semaphore(&state);
			state.frame_sync_primitives[frame_number].render_fence = state.sync_pool.new_fence(&state);
		}
	}

	void VulkanContext::destroy(ExecutionQueue& deletion_queue)
	{
		deletion_queue.flush();

		state.descriptor_set_allocator->destroy(state.get_device());
		state.buffer_allocator->destroy();

		for (int16_t frame_number = MAX_BUFFERED_FRAMES - 1; frame_number >= 0; --frame_number)
		{
			state.sync_pool.release_fence(&state, state.frame_sync_primitives[frame_number].render_fence);
			state.sync_pool.release_semaphore(&state, state.frame_sync_primitives[frame_number].present_semaphore);
			state.sync_pool.release_semaphore(&state, state.frame_sync_primitives[frame_number].render_semaphore);
		}


		vkDestroySurfaceKHR(state.instance, state.surface, nullptr);
		vkDestroyDevice(state.get_device(), nullptr);
		vkb::destroy_debug_utils_messenger(state.instance, state.debug_messenger);
		vkDestroyInstance(state.instance, nullptr);
	}

	void VulkanContext::wait_for_gpu()
	{
		const int16_t current_buffered_frame = get_buffered_frame_number();
		VK_CHECK(vkWaitForFences(state.get_device(), 1, &state.sync_pool.get_fence(state.frame_sync_primitives[current_buffered_frame].render_fence), true, 1000000000));
		VK_CHECK(vkResetFences(state.get_device(), 1, &state.sync_pool.get_fence(state.frame_sync_primitives[current_buffered_frame].render_fence)));
	}

	void VulkanContext::draw(void* buffer, uint32_t vertex_count, uint32_t instance_count)
	{
		vkCmdDraw(static_cast<VkCommandBuffer>(buffer), vertex_count, instance_count, 0, 0);
	}

	void VulkanContext::push_constants(void* buffer, PipelineStateID pipeline_state, const PushConstantPipelineData& push_constant_data)
	{
		VkCommandBuffer command_buffer = static_cast<VkCommandBuffer>(buffer);
		PipelineState* const pso = PipelineStateCache::get()->fetch(pipeline_state);
		assert(pso != nullptr && "Cannot push constants to a null pipeline state");

		VkPipelineLayout pipeline_layout = static_cast<VkPipelineLayout>(pso->get_state_data().layout->get_data());
		assert(pipeline_layout != nullptr && "Cannot push constants to a pipeline state with a null pipeline layout object");

		vkCmdPushConstants(command_buffer, pipeline_layout, VK_FROM_SUNSET_SHADER_STAGE_TYPE(push_constant_data.shader_stage), push_constant_data.offset, static_cast<uint32_t>(push_constant_data.size), push_constant_data.data);
	}

	void VulkanContext::push_descriptor_writes(const std::vector<DescriptorWrite>& descriptor_writes)
	{
		std::vector<VkDescriptorBufferInfo> vk_buffer_infos;
		std::vector<VkDescriptorImageInfo> vk_image_infos;
		std::vector<VkWriteDescriptorSet> vk_writes;

		for (const DescriptorWrite& write : descriptor_writes)
		{
			if (write.type == DescriptorType::UniformBuffer)
			{
				VkDescriptorBufferInfo& buffer_info = vk_buffer_infos.emplace_back();
				buffer_info.buffer = static_cast<VkBuffer>(write.buffer);
				buffer_info.offset = 0;
				buffer_info.range = write.buffer_size;

				VkWriteDescriptorSet& new_vk_write = vk_writes.emplace_back();
				new_vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				new_vk_write.pNext = nullptr;
				new_vk_write.descriptorCount = write.count;
				new_vk_write.descriptorType = VK_FROM_SUNSET_DESCRIPTOR_TYPE(write.type);
				new_vk_write.pBufferInfo = &buffer_info;
				new_vk_write.dstBinding = write.slot;
				new_vk_write.dstSet = static_cast<VkDescriptorSet>(write.set->get());
			}
			else
			{
				VkDescriptorImageInfo& image_info = vk_image_infos.emplace_back();

				VkWriteDescriptorSet& new_vk_write = vk_writes.emplace_back();
				new_vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				new_vk_write.pNext = nullptr;
				new_vk_write.descriptorCount = write.count;
				new_vk_write.descriptorType = VK_FROM_SUNSET_DESCRIPTOR_TYPE(write.type);
				new_vk_write.pImageInfo = &image_info;
				new_vk_write.dstBinding = write.slot;
				new_vk_write.dstSet = static_cast<VkDescriptorSet>(write.set->get());
			}
		}

		vkUpdateDescriptorSets(state.get_device(), static_cast<uint32_t>(vk_writes.size()), vk_writes.data(), 0, nullptr);
	}

	void create_surface(VulkanContext* const vulkan_context, Window* const window)
	{
#if USE_SDL_WINDOWING
		SDL_Vulkan_CreateSurface(static_cast<SDL_Window*>(window->get_window_handle()), vulkan_context->state.instance, &vulkan_context->state.surface);
#endif
	}
}
