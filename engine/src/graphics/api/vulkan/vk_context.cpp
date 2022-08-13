#include <graphics/api/vulkan/vk_context.h>
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

	void VulkanContext::destroy()
	{
		state.sync_pool.release_fence(&state, state.render_fence);
		state.sync_pool.release_semaphore(&state, state.present_semaphore);
		state.sync_pool.release_semaphore(&state, state.render_semaphore);

		vkDestroyDevice(state.get_device(), nullptr);
		vkDestroySurfaceKHR(state.instance, state.surface, nullptr);
		vkb::destroy_debug_utils_messenger(state.instance, state.debug_messenger);
		vkDestroyInstance(state.instance, nullptr);
	}

	void VulkanContext::wait_for_gpu()
	{
		VK_CHECK(vkWaitForFences(state.get_device(), 1, &state.sync_pool.get_fence(state.render_fence), true, 1000000000));
		VK_CHECK(vkResetFences(state.get_device(), 1, &state.sync_pool.get_fence(state.render_fence)));
	}

	void create_surface(VulkanContext* const vulkan_context, Window* const window)
	{
#if USE_SDL_WINDOWING
		SDL_Vulkan_CreateSurface(static_cast<SDL_Window*>(window->get_window_handle()), vulkan_context->state.instance, &vulkan_context->state.surface);
#endif
	}
}
