#pragma once

#include <common.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/image_types.h>

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

		void copy_from(class GraphicsContext* const gfx_context, void* data)
		{
			image_policy.copy_from(gfx_context, data);
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

		void copy_from(class GraphicsContext* const gfx_context, void* data)
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
		static Image* create(class GraphicsContext* const gfx_context, const AttachmentConfig& config);
	};
}
