#pragma once

#include <vk_types.h>
#include <minimal.h>
#include <graphics/resource/image_types.h>

namespace Sunset
{
	class VulkanImage
	{
	public:
		VulkanImage() = default;

		void initialize(class GraphicsContext* const gfx_context, AttachmentConfig& config);
		void initialize(class GraphicsContext* const gfx_context, const AttachmentConfig& config, void* image_handle, void* image_view_handle);
		void destroy(class GraphicsContext* const gfx_context);
		void copy_from_buffer(class GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, class Buffer* buffer, uint32_t buffer_offset = 0, uint32_t mip_level = 0, uint32_t array_layer = 0, uint32_t array_count = 1);
		void copy_to_buffer(class GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, class Buffer* buffer, uint32_t buffer_offset = 0, uint32_t mip_level = 0, uint32_t array_layer = 0, uint32_t array_count = 1);
		char* map_gpu(class GraphicsContext* const gfx_context);
		void unmap_gpu(class GraphicsContext* const gfx_context);
		void bind(class GraphicsContext* const gfx_context, void* command_buffer);
		void barrier(class GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, AccessFlags src_access, AccessFlags dst_access, ImageLayout src_layout, ImageLayout dst_layout, PipelineStageType src_pipeline_stage, PipelineStageType dst_pipeline_stage);
		void blit(class GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, class Image* const other, const glm::vec3 src_blit_size, const glm::vec3 dst_blit_size, uint32_t src_mip = 0, uint32_t dst_mip = 0, int32_t src_layer = -1, int32_t dst_layer = -1);
		void clear(class GraphicsContext* const gfx_context, void* command_buffer, const AttachmentConfig& config, const glm::vec4& clear_color);

		void* get_image()
		{
			return image;
		}

		uint32_t get_num_image_views() const
		{
			return image_views.size();
		}

		void* get_image_view(uint32_t index = 0)
		{
			assert(index >= 0 && index < image_views.size());
			return image_views[index];
		}

		void* get_sampler()
		{
			return sampler;
		}

		AccessFlags get_access_flags() const
		{
			return access_flags;
		}

		ImageLayout get_layout() const
		{
			return layout;
		}

		void set_access_flags(AccessFlags access)
		{
			access_flags = access;
		}

		void set_layout(ImageLayout new_layout)
		{
			layout = new_layout;
		}

	protected:
		VkImage image;
		std::vector<VkImageView> image_views;
		VkSampler sampler;
		AccessFlags access_flags;
		ImageLayout layout;
		VmaAllocation allocation;
		bool b_external_handling{ false };
	};
}
