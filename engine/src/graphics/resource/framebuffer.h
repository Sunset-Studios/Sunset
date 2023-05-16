#pragma once

#include <minimal.h>
#include <common.h>
#include <resource_cache.h>
#include <utility/maths.h>

namespace Sunset
{
	inline std::size_t hash_attachments_list(const std::vector<RenderPassAttachmentInfo>& attachments)
	{
		std::size_t hash = 0;
		for (const RenderPassAttachmentInfo& attachment : attachments)
		{
			hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(attachment.image));
			hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(attachment.array_index));
		}
		return hash;
	}

	template<class Policy>
	class GenericFramebuffer
	{
	public:
		GenericFramebuffer() = default;

		void initialize(class GraphicsContext* const gfx_context, void* render_pass_handle = nullptr, const std::vector<RenderPassAttachmentInfo>& attachments = {})
		{
			framebuffer_policy.initialize(gfx_context, render_pass_handle, attachments);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			framebuffer_policy.destroy(gfx_context);
		}

		void* get_framebuffer_handle()
		{
			return framebuffer_policy.get_framebuffer_handle();
		}

		glm::vec2 get_framebuffer_extent()
		{
			return framebuffer_policy.get_framebuffer_extent();
		}

	private:
		Policy framebuffer_policy;
	};

	class NoopFramebuffer
	{
	public:
		NoopFramebuffer() = default;

		void initialize(class GraphicsContext* const gfx_context, void* render_pass_handle = nullptr, const std::vector<RenderPassAttachmentInfo>& attachments = {})
		{ }

		void destroy(class GraphicsContext* const gfx_context)
		{ }

		void* get_framebuffer_handle()
		{
			return nullptr;
		}

		glm::vec2 get_framebuffer_extent()
		{
			return glm::vec2(0.0f, 0.0f);
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
		static FramebufferID create(class GraphicsContext* const gfx_context, void* render_pass_handle = nullptr, const std::vector<RenderPassAttachmentInfo>& attachments = {}, bool b_auto_delete = false);
	};

	DEFINE_RESOURCE_CACHE(FramebufferCache, FramebufferID, Framebuffer);
}
