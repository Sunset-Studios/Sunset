#include <graphics/api/vulkan/vk_context.h>
#include <graphics/resource/buffer.h>
#include <graphics/pipeline_state.h>
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

		state.render_semaphore = state.sync_pool.new_semaphore(&state);
		state.present_semaphore = state.sync_pool.new_semaphore(&state);
		state.render_fence = state.sync_pool.new_fence(&state); 
	}

	void VulkanContext::destroy(ExecutionQueue& deletion_queue)
	{
		deletion_queue.flush();

		state.buffer_allocator->destroy();

		state.sync_pool.release_fence(&state, state.render_fence);
		state.sync_pool.release_semaphore(&state, state.present_semaphore);
		state.sync_pool.release_semaphore(&state, state.render_semaphore);

		vkDestroySurfaceKHR(state.instance, state.surface, nullptr);
		vkDestroyDevice(state.get_device(), nullptr);
		vkb::destroy_debug_utils_messenger(state.instance, state.debug_messenger);
		vkDestroyInstance(state.instance, nullptr);
	}

	void VulkanContext::wait_for_gpu()
	{
		VK_CHECK(vkWaitForFences(state.get_device(), 1, &state.sync_pool.get_fence(state.render_fence), true, 1000000000));
		VK_CHECK(vkResetFences(state.get_device(), 1, &state.sync_pool.get_fence(state.render_fence)));
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

	void create_surface(VulkanContext* const vulkan_context, Window* const window)
	{
#if USE_SDL_WINDOWING
		SDL_Vulkan_CreateSurface(static_cast<SDL_Window*>(window->get_window_handle()), vulkan_context->state.instance, &vulkan_context->state.surface);
#endif
	}
}
