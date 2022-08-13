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

		void request_next_image(class GraphicsContext* const gfx_context)
		{
			swapchain_policy.request_next_image(gfx_context);
		}

		void present(class GraphicsContext* const gfx_context, class GraphicsCommandQueue* const command_queue)
		{
			swapchain_policy.present(gfx_context, command_queue);
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

		void request_next_image(class GraphicsContext* const gfx_context)
		{ }

		void present(class GraphicsContext* const gfx_context, class GraphicsCommandQueue* const command_queue)
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
