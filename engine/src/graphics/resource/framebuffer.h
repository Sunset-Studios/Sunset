#pragma once

#include <common.h>

namespace Sunset
{
	template<class Policy>
	class GenericFramebuffer
	{
	public:
		GenericFramebuffer() = default;

		void initialize(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* render_pass_handle = nullptr, void* attachments_handle = nullptr)
		{
			framebuffer_policy.initialize(gfx_context, swapchain, render_pass_handle, attachments_handle);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			framebuffer_policy.destroy(gfx_context);
		}

		void* get_framebuffer_handle()
		{
			return framebuffer_policy.get_framebuffer_handle();
		}

	private:
		Policy framebuffer_policy;
	};

	class NoopFramebuffer
	{
	public:
		NoopFramebuffer() = default;

		void initialize(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, void* render_pass_handle = nullptr, void* attachments_handle = nullptr)
		{ }

		void destroy(class GraphicsContext* const gfx_context)
		{ }

		void* get_framebuffer_handle()
		{
			return nullptr;
		}
	};

#if USE_VULKAN_GRAPHICS
	class Framebuffer : public GenericFramebuffer<VulkanFramebuffer>
	{ };
#else
	class Framebuffer : public GenericFramebuffer<NoopFramebuffer>
	{ };
#endif

	class FramebufferFactory
	{
	public:
		template<typename ...Args>
		static Framebuffer* create(Args&&... args)
		{
			Framebuffer* gfx = new Framebuffer;
			gfx->initialize(std::forward<Args>(args)...);
			return gfx;
		}
	};
}
