#pragma once

#include <minimal.h>
#include <vk_types.h>

namespace Sunset
{
	class VulkanBarrierBatcher
	{
	public:
		VulkanBarrierBatcher() = default;

		void begin(class GraphicsContext* const gfx_context, PipelineStageType stage);
		void add_buffer_barrier(class GraphicsContext* const gfx_context, class Buffer* buffer, Identity execution_id, AccessFlags destination_access);
		void add_image_barrier(class GraphicsContext* const gfx_context, class Image* image, Identity execution_id, AccessFlags destination_access, ImageLayout final_layout);
		void execute(class GraphicsContext* const gfx_context, void* command_buffer, PipelineStageType stage, Identity execution_id = "");
		void reset();

	private:
		PipelineStageType current_stage;
		std::unordered_map<Identity, std::vector<VkBufferMemoryBarrier>> buffer_barriers;
		std::unordered_map<Identity, std::vector<VkImageMemoryBarrier>> image_barriers;
	};
}
