#include <graphics/resource/image.h>
#include <graphics/asset_pool.h>
#include <graphics/graphics_context.h>
#include <graphics/renderer.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Sunset
{
	Sunset::Image* ImageFactory::create(class GraphicsContext* const gfx_context, const AttachmentConfig& config, bool auto_delete)
	{
		Image* image = GlobalAssetPools<Image>::get()->allocate();
		image->initialize(gfx_context, config);
		if (auto_delete)
		{
			gfx_context->add_resource_deletion_execution([image, gfx_context]()
			{
				image->destroy(gfx_context);
				GlobalAssetPools<Image>::get()->deallocate(image);
			});
		}
		return image;
	}

	Sunset::Image* ImageFactory::load(class GraphicsContext* const gfx_context, const char* path)
	{
		ImageID image_id = ImageCache::get()->fetch_or_add(path, gfx_context);
		Image* image = ImageCache::get()->fetch(image_id);

		if (image->get_image() == nullptr)
		{
			int texture_width, texture_height, texture_channels;

			stbi_uc* const pixel_buffer = stbi_load(path, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
			if (pixel_buffer == nullptr)
			{
				return nullptr;
			}

			const size_t image_size = texture_height * texture_width * 4;
			Buffer* const staging_buffer = BufferFactory::create(gfx_context, image_size, BufferType::TransferSource, MemoryUsageType::OnlyCPU, false);

			staging_buffer->copy_from(gfx_context, pixel_buffer, image_size);

			stbi_image_free(pixel_buffer);

			{
				AttachmentConfig config;
				config.format = Format::SRGB8x4;
				config.extent = glm::vec3(texture_width, texture_height, 1);
				config.flags = (ImageFlags::Sampled | ImageFlags::TransferDst);
				config.usage_type = MemoryUsageType::OnlyGPU;
				config.image_filter = ImageFilter::Nearest;
				image->initialize(gfx_context, config);

				Renderer::get()->graphics_command_queue()->submit_immediate(gfx_context, [image, staging_buffer, gfx_context](void* command_buffer)
				{
					image->copy_buffer(gfx_context, command_buffer, staging_buffer);
				});
			}

			staging_buffer->destroy(gfx_context);
			GlobalAssetPools<Buffer>::get()->deallocate(staging_buffer);
		}

		return image;
	}

	void ImageCache::initialize()
	{
	}

	void ImageCache::update()
	{
	}

	Sunset::ImageID ImageCache::fetch_or_add(const char* file_path, class GraphicsContext* const gfx_context /*= nullptr*/)
	{
		ImageID id = std::hash<std::string>{}(file_path);
		if (cache.find(id) == cache.end())
		{
			Image* const new_image = GlobalAssetPools<Image>::get()->allocate();
			gfx_context->add_resource_deletion_execution([new_image, gfx_context]()
			{ 
				new_image->destroy(gfx_context);
				GlobalAssetPools<Image>::get()->deallocate(new_image);
			});
			cache.insert({ id, new_image });
		}
		return id;
	}

	void ImageCache::remove(ImageID id)
	{
		cache.erase(id);
	}

	Sunset::Image* ImageCache::fetch(ImageID id)
	{
		assert(cache.find(id) != cache.end());
		return cache[id];
	}

	void ImageCache::destroy(class GraphicsContext* const gfx_context)
	{
		for (const std::pair<size_t, Image*>& pair : cache)
		{
			pair.second->destroy(gfx_context);
		}
		cache.clear();
	}
}
