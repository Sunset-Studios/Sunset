#pragma once

#include <common.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/image_types.h>
#include <graphics/resource/resource_cache.h>

namespace Sunset
{
	template<class Policy>
	class GenericImage
	{
	public:
		GenericImage() = default;

		void initialize(class GraphicsContext* const gfx_context, const AttachmentConfig& config)
		{
			attachment_config = config;
			image_policy.initialize(gfx_context, attachment_config);
		}

		void copy_buffer(class GraphicsContext* const gfx_context, void* command_buffer, class Buffer* buffer)
		{
			image_policy.copy_buffer(gfx_context, command_buffer, attachment_config, buffer);
		}

		void bind(class GraphicsContext* const gfx_context, void* command_buffer)
		{
			image_policy.bind(gfx_context, command_buffer);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			image_policy.destroy(gfx_context);
		}

		AttachmentConfig& get_attachment_config()
		{
			return attachment_config;
		}

		void* get_image()
		{
			return image_policy.get_image();
		}

		void* get_image_view()
		{
			return image_policy.get_image_view();
		}

		void* get_sampler()
		{
			return image_policy.get_sampler();
		}

	private:
		Policy image_policy;
		AttachmentConfig attachment_config;
	};

	class NoopImage
	{
	public:
		NoopImage() = default;

		void initialize(class GraphicsContext* const gfx_context, AttachmentConfig& config)
		{ }

		void destroy(class GraphicsContext* const gfx_context)
		{ }

		void copy_buffer(class GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, class Buffer* buffer)
		{ }

		void bind(class GraphicsContext* const gfx_context, void* command_buffer)
		{ }

		void* get_image()
		{
			return nullptr;
		}

		void* get_image_view()
		{
			return nullptr;
		}

		void* get_sampler()
		{
			return nullptr;
		}
	};

#if USE_VULKAN_GRAPHICS
	class Image : public GenericImage<VulkanImage>
	{ };
#else
	class Image : public GenericImage<NoopImage>
	{ };
#endif

	class ImageFactory
	{
	public:
		static ImageID create(class GraphicsContext* const gfx_context, const AttachmentConfig& config, bool auto_delete = true);
		static ImageID load(class GraphicsContext* const gfx_context, const AttachmentConfig& config);
	};

	DEFINE_RESOURCE_CACHE(ImageCache, ImageID, Image);
}
