#include <graphics/api/vulkan/vk_context.h>
#include <graphics/command_queue.h>
#include <graphics/resource/buffer.h>
#include <graphics/pipeline_state.h>
#include <graphics/descriptor.h>
#include <graphics/resource/image.h>
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

		VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features = {};
		shader_draw_parameters_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
		shader_draw_parameters_features.pNext = nullptr;
		shader_draw_parameters_features.shaderDrawParameters = VK_TRUE;

		state.device = device_builder.add_pNext(&shader_draw_parameters_features).build().value();
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

	void VulkanContext::draw(void* buffer, uint32_t vertex_count, uint32_t instance_count, uint32_t instance_index)
	{
		vkCmdDraw(static_cast<VkCommandBuffer>(buffer), vertex_count, instance_count, 0, instance_index);
	}

	void VulkanContext::draw_indexed(void* buffer, uint32_t index_count, uint32_t instance_count, uint32_t instance_index /*= 0*/)
	{
		vkCmdDrawIndexed(static_cast<VkCommandBuffer>(buffer), index_count, instance_count, instance_index, 0, 0);
	}

	void VulkanContext::draw_indexed_indirect(void* buffer, class Buffer* indirect_buffer, uint32_t draw_count, uint32_t draw_first /*= 0*/)
	{
		VkDeviceSize indirect_offset = draw_first * sizeof(VulkanGPUIndirectObject);
		uint32_t stride = sizeof(VulkanGPUIndirectObject);
		vkCmdDrawIndexedIndirect(static_cast<VkCommandBuffer>(buffer), static_cast<VkBuffer>(indirect_buffer->get()), indirect_offset, draw_count, stride);
	}

	void VulkanContext::register_command_queue(DeviceQueueType queue_type)
	{
		const int16_t queue_type_idx = static_cast<int16_t>(queue_type);
		if (state.queues[queue_type_idx] == nullptr)
		{
			state.queues[queue_type_idx] = std::move(GraphicsCommandQueueFactory::create(&state, queue_type));
		}
	}

	void VulkanContext::destroy_command_queue(DeviceQueueType queue_type)
	{
		const int16_t queue_type_idx = static_cast<int16_t>(queue_type);
		if (state.queues[queue_type_idx] != nullptr)
		{
			state.queues[queue_type_idx]->destroy(&state);
		}
	}

	CommandQueue* VulkanContext::get_command_queue(DeviceQueueType queue_type)
	{
		return state.queues[static_cast<int16_t>(queue_type)].get();
	}

	void VulkanContext::push_constants(void* buffer, PipelineStateID pipeline_state, const PushConstantPipelineData& push_constant_data)
	{
		VkCommandBuffer command_buffer = static_cast<VkCommandBuffer>(buffer);
		PipelineState* const pso = CACHE_FETCH(PipelineState, pipeline_state);
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

		vk_buffer_infos.reserve(descriptor_writes.size());
		vk_image_infos.reserve(descriptor_writes.size());

		for (const DescriptorWrite& write : descriptor_writes)
		{
			if (write.type == DescriptorType::Image)
			{
				Image* const image = static_cast<Image*>(write.buffer);

				VkDescriptorImageInfo& image_info = vk_image_infos.emplace_back();
				image_info.sampler = static_cast<VkSampler>(image->get_sampler());
				image_info.imageView = static_cast<VkImageView>(image->get_image_view());
				image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				VkWriteDescriptorSet& new_vk_write = vk_writes.emplace_back();
				new_vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				new_vk_write.pNext = nullptr;
				new_vk_write.descriptorCount = write.count;
				new_vk_write.descriptorType = VK_FROM_SUNSET_DESCRIPTOR_TYPE(write.type);
				new_vk_write.pImageInfo = &image_info;
				new_vk_write.dstBinding = write.slot;
				new_vk_write.dstSet = static_cast<VkDescriptorSet>(write.set->get());
			}
			else
			{
				VkDescriptorBufferInfo& buffer_info = vk_buffer_infos.emplace_back();
				buffer_info.buffer = static_cast<VkBuffer>(write.buffer);
				buffer_info.offset = 0;
				buffer_info.range = write.buffer_range;

				VkWriteDescriptorSet& new_vk_write = vk_writes.emplace_back();
				new_vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				new_vk_write.pNext = nullptr;
				new_vk_write.descriptorCount = write.count;
				new_vk_write.descriptorType = VK_FROM_SUNSET_DESCRIPTOR_TYPE(write.type);
				new_vk_write.pBufferInfo = &buffer_info;
				new_vk_write.dstBinding = write.slot;
				new_vk_write.dstSet = static_cast<VkDescriptorSet>(write.set->get());
			}
		}

		vkUpdateDescriptorSets(state.get_device(), static_cast<uint32_t>(vk_writes.size()), vk_writes.data(), 0, nullptr);
	}

	size_t VulkanContext::get_min_ubo_offset_alignment()
	{
		return state.device.physical_device.properties.limits.minUniformBufferOffsetAlignment;
	}

	void VulkanContext::update_indirect_draw_command(void* commands, uint32_t command_index, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint64_t object_id, uint32_t batch_id)
	{
		VulkanGPUIndirectObject* vk_commands = static_cast<VulkanGPUIndirectObject*>(commands);
		vk_commands[command_index].indirect_command.indexCount = index_count;
		vk_commands[command_index].indirect_command.firstIndex = first_index;
		vk_commands[command_index].indirect_command.instanceCount = instance_count;
		vk_commands[command_index].indirect_command.firstInstance = first_instance;
		vk_commands[command_index].object_instance.object_id = object_id;
		vk_commands[command_index].object_instance.batch_id = batch_id;
	}

	void create_surface(VulkanContext* const vulkan_context, Window* const window)
	{
#if USE_SDL_WINDOWING
		SDL_Vulkan_CreateSurface(static_cast<SDL_Window*>(window->get_window_handle()), vulkan_context->state.instance, &vulkan_context->state.surface);
#endif
	}
}
