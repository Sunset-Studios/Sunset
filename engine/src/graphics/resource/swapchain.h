#pragma once

#include <common.h>

namespace Sunset
{
	template<class Policy>
	class GenericSwapchain
	{
	public:
		GenericSwapchain() = default;

		void initialize(class GraphicsContext* const gfx_context)
		{
			swapchain_policy.initialize(gfx_context);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			swapchain_policy.destroy(gfx_context);
		}

		void* get_data()
		{
			return swapchain_policy.get_data();
		}

		Format get_format()
		{
			return swapchain_policy.get_format();
		}

		uint32_t get_current_image_index()
		{
			return swapchain_policy.get_current_image_index();
		}

		void request_next_image(class GraphicsContext* const gfx_context)
		{
			swapchain_policy.request_next_image(gfx_context);
		}

		void present(class GraphicsContext* const gfx_context, DeviceQueueType queue_type)
		{
			swapchain_policy.present(gfx_context, queue_type);
		}

	private:
		Policy swapchain_policy;
	};

	class NoopSwapchain
	{
	public:
		NoopSwapchain() = default;

		void initialize(class GraphicsContext* const gfx_context)
		{ }

		void destroy(class GraphicsContext* const gfx_context)
		{ }

		void* get_data()
		{
			return nullptr;
		}

		Format get_format()
		{
			return Format::Undefined;
		}

		uint32_t get_current_image_index()
		{
			return 0;
		}

		void request_next_image(class GraphicsContext* const gfx_context)
		{ }

		void present(class GraphicsContext* const gfx_context, DeviceQueueType queue_type)
		{ }
	};

#if USE_VULKAN_GRAPHICS
	class Swapchain : public GenericSwapchain<VulkanSwapchain>
	{ };
#else
	class Swapchain : public GenericSwapchain<NoopSwapchain>
	{ };
#endif

	class SwapchainFactory
	{
	public:
		template<typename ...Args>
		static Swapchain* create(Args&&... args)
		{
			Swapchain* gfx = new Swapchain;
			gfx->initialize(std::forward<Args>(args)...);
			return gfx;
		}
	};
}
