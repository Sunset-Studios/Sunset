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

		void initialize(class GraphicsContext* const gfx_context, const AttachmentConfig& config, void* image_handle, void* image_view_handle)
		{
			attachment_config = config;
			image_policy.initialize(gfx_context, attachment_config, image_handle, image_view_handle);
		}

		void copy_from_buffer(class GraphicsContext* const gfx_context, void* command_buffer, class Buffer* buffer)
		{
			image_policy.copy_from_buffer(gfx_context, command_buffer, attachment_config, buffer);
		}

		void copy_to_buffer(class GraphicsContext* const gfx_context, void* command_buffer, class Buffer* buffer, uint32_t buffer_offset = 0, uint32_t mip_level = 0, uint32_t array_layer = 0)
		{
			image_policy.copy_to_buffer(gfx_context, command_buffer, attachment_config, buffer, buffer_offset, mip_level, array_layer);
		}

		char* map_gpu(class GraphicsContext* const gfx_context)
		{
			return image_policy.map_gpu(gfx_context);
		}

		void unmap_gpu(class GraphicsContext* const gfx_context)
		{
			image_policy.unmap_gpu(gfx_context);
		}

		void bind(class GraphicsContext* const gfx_context, void* command_buffer)
		{
			image_policy.bind(gfx_context, command_buffer);
		}

		void barrier(class GraphicsContext* const gfx_context, void* command_buffer, AccessFlags src_access, AccessFlags dst_access, ImageLayout src_layout, ImageLayout dst_layout, PipelineStageType src_pipeline_stage, PipelineStageType dst_pipeline_stage)
		{
			image_policy.barrier(gfx_context, command_buffer, attachment_config, src_access, dst_access, src_layout, dst_layout, src_pipeline_stage, dst_pipeline_stage);
		}

		void blit(class GraphicsContext* const gfx_context, void* command_buffer, Image* const other, const glm::vec3 src_blit_size, const glm::vec3 dst_blit_size, uint32_t src_mip = 0, uint32_t dst_mip = 0, int32_t src_layer = -1, int32_t dst_layer = -1)
		{
			image_policy.blit(gfx_context, command_buffer, attachment_config, other, src_blit_size, dst_blit_size, src_mip, dst_mip, src_layer, dst_layer);
		}

		void clear(class GraphicsContext* const gfx_context, void* command_buffer, const glm::vec4& clear_color)
		{
			image_policy.clear(gfx_context, command_buffer, attachment_config, clear_color);
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

		uint32_t get_num_image_views() const
		{
			return image_policy.get_num_image_views();
		}

		void* get_image_view(uint32_t index = 0)
		{
			return image_policy.get_image_view(index);
		}

		void* get_sampler()
		{
			return image_policy.get_sampler();
		}

		AccessFlags get_access_flags() const
		{
			return image_policy.get_access_flags();
		}

		ImageLayout get_layout() const
		{
			return image_policy.get_layout();
		}

		void set_access_flags(AccessFlags access)
		{
			image_policy.set_access_flags(access);
		}

		void set_layout(ImageLayout new_layout)
		{
			image_policy.set_layout(new_layout);
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

		void initialize(class GraphicsContext* const gfx_context, const AttachmentConfig& config, void* image_handle, void* image_view_handle)
		{ }

		void destroy(class GraphicsContext* const gfx_context)
		{ }

		void copy_from_buffer(class GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, class Buffer* buffer)
		{ }

		void copy_to_buffer(class GraphicsContext* const gfx_context, void* command_buffer, class Buffer* buffer, uint32_t buffer_offset = 0, uint32_t mip_level = 0, uint32_t array_layer = 0)
		{ }

		char* map_gpu(class GraphicsContext* const gfx_context)
		{ }

		void unmap_gpu(class GraphicsContext* const gfx_context)
		{ }

		void bind(class GraphicsContext* const gfx_context, void* command_buffer)
		{ }

		void barrier(class GraphicsContext* const gfx_context, void* command_buffer, AccessFlags src_access, AccessFlags dst_access, ImageLayout src_layout, ImageLayout dst_layout, PipelineStageType src_pipeline_stage, PipelineStageType dst_pipeline_stage)
		{ }

		void blit(class GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, Image* const other, const glm::vec3 src_blit_size, const glm::vec3 dst_blit_size, uint32_t src_mip = 0, uint32_t dst_mip = 0, int32_t src_layer = -1, int32_t dst_layer = -1)
		{ }

		void clear(class GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, const glm::vec4& clear_color)
		{ }

		void* get_image()
		{
			return nullptr;
		}

		uint32_t get_num_image_views() const
		{
			return 0;
		}

		void* get_image_view(uint32_t index = 0)
		{
			return nullptr;
		}

		void* get_sampler()
		{
			return nullptr;
		}

		AccessFlags get_access_flags() const
		{
			return AccessFlags::None;
		}

		ImageLayout get_layout() const
		{
			return ImageLayout::Undefined;
		}

		void set_access_flags(AccessFlags access)
		{ }

		void set_layout(ImageLayout new_layout)
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
		static ImageID create(class GraphicsContext* const gfx_context, const AttachmentConfig& config, void* image_handle, void* image_view_handle, bool auto_delete = true);
		static ImageID create(class GraphicsContext* const gfx_context, const AttachmentConfig& config, bool auto_delete = true);
		static ImageID create_default(class GraphicsContext* const gfx_context);
		static ImageID load(class GraphicsContext* const gfx_context, const AttachmentConfig& config);
	};

	DEFINE_RESOURCE_CACHE(ImageCache, ImageID, Image);

	struct ScopedGPUImageMapping
	{
		ScopedGPUImageMapping(class GraphicsContext* const gfx_context, Image* image);
		~ScopedGPUImageMapping();

		class GraphicsContext* gfx_context{ nullptr };
		char* mapped_memory{ nullptr };
		Image* image{ nullptr };
	};
}
