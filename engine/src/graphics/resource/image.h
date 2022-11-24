#pragma once

#include <common.h>
#include <graphics/resource/buffer.h>

namespace Sunset
{
	template<class Policy>
	class GenericImage
	{
	public:
		GenericImage() = default;

		void initialize(class GraphicsContext* const gfx_context, Format format, ImageType image_type, const glm:vec3& extent, ImageUsage usage)
		{
			image_policy.initialize(gfx_context, format, image_type, extent, usage);
		}

		void copy_from(class GraphicsContext* const gfx_context, void* data)
		{
			image_policy.copy_from(gfx_context, data);
		}

		void bind(class GraphicsContext* const gfx_context, void* command_buffer)
		{
			image_policy.bind(gfx_context, buffer_type, command_buffer);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			image_policy.destroy(gfx_context);
		}

	private:
		Policy image_policy;
	};

	class NoopImage
	{
	public:
		NoopImage() = default;

		void initialize(class GraphicsContext* const gfx_context, Format format, ImageType image_type, const glm::vec3 & extent, ImageUsage usage)
		{ }

		void destroy(class GraphicsContext* const gfx_context)
		{ }

		void copy_from(class GraphicsContext* const gfx_context, void* data)
		{ }

		void bind(class GraphicsContext* const gfx_context, void* command_buffer)
		{ }
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
		static Image* create(class GraphicsContext* const gfx_context, Format format, ImageType image_type, const glm::vec3 & extent, ImageUsage usage);
	};
}
