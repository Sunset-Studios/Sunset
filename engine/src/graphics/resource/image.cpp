#include <graphics/resource/image.h>
#include <graphics/asset_pool.h>
#include <graphics/graphics_context.h>
#include <graphics/renderer.h>
#include <graphics/command_queue.h>
#include <image_serializer.h>

#include <filesystem>

namespace Sunset
{
	Sunset::ImageID ImageFactory::create(class GraphicsContext* const gfx_context, const AttachmentConfig& config, bool auto_delete)
	{
		bool b_added{ false };
		ImageID image_id = ImageCache::get()->fetch_or_add(config.name, gfx_context, b_added, auto_delete);
		if (b_added)
		{
			Image* image = CACHE_FETCH(Image, image_id);
			image->initialize(gfx_context, config);
		}
		return image_id;
	}

	Sunset::ImageID ImageFactory::create(class GraphicsContext* const gfx_context, const AttachmentConfig& config, void* image_handle, void* image_view_handle, bool auto_delete /*= true*/)
	{
		bool b_added{ false };
		ImageID image_id = ImageCache::get()->fetch_or_add(config.name, gfx_context, b_added, auto_delete);
		if (b_added)
		{
			Image* image = CACHE_FETCH(Image, image_id);
			image->initialize(gfx_context, config, image_handle, image_view_handle);
		}
		return image_id;
	}

	Sunset::ImageID ImageFactory::create_default(class GraphicsContext* const gfx_context)
	{
		static ImageID image;
		if (image == 0)
		{
			AttachmentConfig config;
			config.name = "default";
			config.format = Format::Float4x16;
			config.extent = glm::vec3(1.0f, 1.0f, 1.0f);
			config.flags = ImageFlags::Color | ImageFlags::Sampled | ImageFlags::Storage;
			config.usage_type = MemoryUsageType::OnlyGPU;
			config.sampler_address_mode = SamplerAddressMode::Repeat;
			config.image_filter = ImageFilter::Linear;
			image = create(gfx_context, config);
		}
		return image;
	}

	Sunset::ImageID ImageFactory::load(class GraphicsContext* const gfx_context, const AttachmentConfig& config)
	{
		bool b_added{ false };
		ImageID image_id = ImageCache::get()->fetch_or_add(config.name, gfx_context, b_added);
		Image* image = CACHE_FETCH(Image, image_id);

		if (b_added)
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
				image_config.flags |= ImageFlags::TransferDst;
				image_config.format = image_format;
				image_config.extent = glm::vec3(image_info.extent[0], image_info.extent[1], image_info.extent[2]);
				image->initialize(gfx_context, image_config);

				gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(gfx_context, [image, staging_buffer, gfx_context](void* command_buffer)
				{
					image->copy_from_buffer(gfx_context, command_buffer, staging_buffer);
				});
			}

			CACHE_DELETE(Buffer, staging_buffer_id, gfx_context);
		}

		return image_id;
	}

	ImageID ImageFactory::load_cubemap(GraphicsContext* const gfx_context, const AttachmentConfig& config)
	{
		bool b_added{ false };
		ImageID image_id = ImageCache::get()->fetch_or_add(config.name, gfx_context, b_added);
		Image* image = CACHE_FETCH(Image, image_id);

		if (b_added)
		{
			BufferID staging_buffer_id{ 0 };
			Buffer* staging_buffer{ nullptr };
			uint32_t buffer_offset{ 0 };
			std::vector<uint32_t> mip_buffer_offsets(config.mip_count, 0);

			const std::filesystem::path cubemap_path{ config.path };
			std::string cubemap_face_path = std::format("{}/{}{}", cubemap_path.parent_path().string(), cubemap_path.stem().string(), "_mip_0_layer_0.sun");

			size_t image_size = 0;
			Format image_format = Format::Undefined;
			uint32_t image_extent[3] = { 0, 0, 0 };

			for (uint32_t j = 0; j < config.mip_count; ++j)
			{
				cubemap_face_path[cubemap_face_path.size() - 13] = '0' + j;

				mip_buffer_offsets[j] = buffer_offset;

				for (uint32_t i = 0; i < 6; ++i)
				{
					cubemap_face_path[cubemap_face_path.size() - 5] = '0' + i;

					SerializedAsset asset;
					if (!deserialize_asset(cubemap_face_path.c_str(), asset))
					{
						continue;
					}

					SerializedImageInfo image_info = get_serialized_image_info(&asset);
					image_size = image_info.size;
					image_format = image_info.format;

					if (j == 0)
					{
						image_extent[0] = image_info.extent[0];
						image_extent[1] = image_info.extent[1];
						image_extent[2] = image_info.extent[2];
					}

					if (staging_buffer_id == 0)
					{
						staging_buffer_id = BufferFactory::create(
							gfx_context,
							{
								.name = config.path,
								.buffer_size = image_size * 6 * config.mip_count,
								.type = BufferType::TransferSource,
								.memory_usage = MemoryUsageType::OnlyCPU
							},
							false
						);
						staging_buffer = CACHE_FETCH(Buffer, staging_buffer_id);
					}

					staging_buffer->copy_from(gfx_context, asset.binary.data(), asset.binary.size(), 0, [&image_info, &asset, buffer_offset](void* memory)
					{
						unpack_image(&image_info, asset.binary.data(), asset.binary.size(), ((char*)memory) + buffer_offset);
					});

					buffer_offset += image_size;
				}
			}

			AttachmentConfig image_config = config;
			image_config.flags |= (ImageFlags::TransferDst | ImageFlags::Image2DArray);
			image_config.format = image_format;
			image_config.extent = glm::vec3(image_extent[0], image_extent[1], image_extent[2]);
			image_config.array_count = 6;
			image->initialize(gfx_context, image_config);

			gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(gfx_context, [image, staging_buffer, mip_buffer_offsets, image_config, gfx_context](void* command_buffer)
			{
				for (uint32_t i = 0; i < image_config.mip_count; ++i)
				{
					image->copy_from_buffer(gfx_context, command_buffer, staging_buffer, mip_buffer_offsets[i], i, 0, 6);
					image->barrier(
						gfx_context,
						command_buffer,
						AccessFlags::ShaderRead,
						AccessFlags::None,
						ImageLayout::ShaderReadOnly,
						ImageLayout::TransferDestination,
						PipelineStageType::FragmentShader,
						PipelineStageType::TopOfPipe
					);
				}
			});

			CACHE_DELETE(Buffer, staging_buffer_id, gfx_context);
		}

		return image_id;
	}

	ScopedGPUImageMapping::ScopedGPUImageMapping(GraphicsContext* const gfx_context, Image* image)
		: image(image), gfx_context(gfx_context)
	{
		mapped_memory = image->map_gpu(gfx_context);
	}

	ScopedGPUImageMapping::~ScopedGPUImageMapping()
	{
		image->unmap_gpu(gfx_context);
	}
}
