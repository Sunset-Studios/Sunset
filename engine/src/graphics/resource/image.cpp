#include <graphics/resource/image.h>
#include <graphics/asset_pool.h>
#include <graphics/graphics_context.h>
#include <graphics/renderer.h>
#include <graphics/command_queue.h>
#include <image_serializer.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <json.hpp>
#include <lz4.h>

namespace Sunset
{
	Sunset::ImageID ImageFactory::create(class GraphicsContext* const gfx_context, const AttachmentConfig& config, bool auto_delete)
	{
		ImageID image_id = ImageCache::get()->fetch_or_add(config.name, gfx_context);
		Image* image = CACHE_FETCH(Image, image_id);
		image->initialize(gfx_context, config);
		if (auto_delete)
		{
			gfx_context->add_resource_deletion_execution([image_id, image, gfx_context]()
			{
				ImageCache::get()->remove(image_id);
				image->destroy(gfx_context);
				GlobalAssetPools<Image>::get()->deallocate(image);
			});
		}
		return image_id;
	}

	Sunset::ImageID ImageFactory::create(class GraphicsContext* const gfx_context, const AttachmentConfig& config, void* image_handle, void* image_view_handle, bool auto_delete /*= true*/)
	{
		ImageID image_id = ImageCache::get()->fetch_or_add(config.name, gfx_context);
		Image* image = CACHE_FETCH(Image, image_id);
		image->initialize(gfx_context, config, image_handle, image_view_handle);
		if (auto_delete)
		{
			gfx_context->add_resource_deletion_execution([image_id, image, gfx_context]()
			{
				ImageCache::get()->remove(image_id);
				image->destroy(gfx_context);
				GlobalAssetPools<Image>::get()->deallocate(image);
			});
		}
		return image_id;
	}

	Sunset::ImageID ImageFactory::load(class GraphicsContext* const gfx_context, const AttachmentConfig& config)
	{
		ImageID image_id = ImageCache::get()->fetch_or_add(config.name, gfx_context);
		Image* image = CACHE_FETCH(Image, image_id);

		if (image->get_image() == nullptr)
		{
			SerializedAsset asset;
			if (!deserialize_asset(config.path, asset))
			{
				return ImageID();
			}

			SerializedImageInfo image_info = get_serialized_image_info(&asset);

			const size_t image_size = image_info.size;
			const Format image_format = image_info.format;

			const BufferID staging_buffer_id = BufferFactory::create(
				gfx_context, 
				{
					.name = config.path,
					.buffer_size = image_size,
					.type = BufferType::TransferSource,
					.memory_usage = MemoryUsageType::OnlyCPU
				},
				false
			);
			Buffer* const staging_buffer = CACHE_FETCH(Buffer, staging_buffer_id);

			staging_buffer->copy_from(gfx_context, asset.binary.data(), asset.binary.size(), 0, [&image_info, &asset](void* memory)
			{
				unpack_image(&image_info, asset.binary.data(), asset.binary.size(), (char*)memory);
			});

			{
				AttachmentConfig image_config = config;
				image_config.format = image_format;
				image_config.extent = glm::vec3(image_info.extent[0], image_info.extent[1], image_info.extent[2]);
				image->initialize(gfx_context, config);

				gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(gfx_context, [image, staging_buffer, gfx_context](void* command_buffer)
				{
					image->copy_buffer(gfx_context, command_buffer, staging_buffer);
				});
			}

			staging_buffer->destroy(gfx_context);
			GlobalAssetPools<Buffer>::get()->deallocate(staging_buffer);
		}

		return image_id;
	}
}
