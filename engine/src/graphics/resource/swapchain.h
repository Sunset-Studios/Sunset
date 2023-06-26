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

		uint32_t get_current_image_index(int32_t buffered_frame)
		{
			return swapchain_policy.get_current_image_index(buffered_frame);
		}

		void request_next_image(class GraphicsContext* const gfx_context, int32_t buffered_frame)
		{
			swapchain_policy.request_next_image(gfx_context, buffered_frame);
		}

		void present(class GraphicsContext* const gfx_context, DeviceQueueType queue_type, int32_t buffered_frame)
		{
			swapchain_policy.present(gfx_context, queue_type, buffered_frame);
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

		uint32_t get_current_image_index(int32_t buffered_frame)
		{
			return 0;
		}

		void request_next_image(class GraphicsContext* const gfx_context, int32_t buffered_frame)
		{ }

		void present(class GraphicsContext* const gfx_context, DeviceQueueType queue_type, int32_t buffered_frame)
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
