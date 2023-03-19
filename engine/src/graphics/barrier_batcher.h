#pragma once

#include <minimal.h>
#include <pipeline_types.h>
#include <utility/strings.h>

/**
 * Intended usage is to batch memory barriers per pipeline stage using add_buffer_barrier/add_image_barrier
 * and submit + flush the barriers for that pipeline stage using a call to submit. Subsequent calls to submit
 * will remember the previous pipeline stage in order to do proper source-to-destination pipeline stage transitions.
 * This behavior can be reset with a call to reset, which will just set the pipeline stage back to TopOfPipe.
 * This means knowledge of pipeline stages is expected to come from external calling code (exposed as a parameter to execute)
 * and is therefore not encoded in any of the recorded barriers.
 */

namespace Sunset
{
	template<class Policy>
	class GenericBarrierBatcher
	{
	public:
		GenericBarrierBatcher() = default;

		void begin(class GraphicsContext* const gfx_context, PipelineStageType stage)
		{
			batcher_policy.begin(gfx_context, stage);
		}

		void add_buffer_barrier(class GraphicsContext* const gfx_context, class Buffer* buffer, Identity execution_id, AccessFlags destination_access)
		{
			batcher_policy.add_buffer_barrier(gfx_context, buffer, execution_id, destination_access);
		}

		void add_image_barrier(class GraphicsContext* const gfx_context, class Image* image, Identity execution_id, AccessFlags destination_access, ImageLayout final_layout)
		{
			batcher_policy.add_image_barrier(gfx_context, image, execution_id, destination_access, final_layout);
		}

		void execute(class GraphicsContext* const gfx_context, void* command_buffer, PipelineStageType stage, Identity execution_id = "")
		{
			batcher_policy.execute(gfx_context, command_buffer, stage, execution_id);
		}

		void reset()
		{
			batcher_policy.reset();
		}

	private:
		Policy batcher_policy;
	};

	class NoopBarrierBatcher
	{
	public:
		NoopBarrierBatcher() = default;

		void begin(class GraphicsContext* const gfx_context, PipelineStageType stage)
		{ }

		void add_buffer_barrier(class GraphicsContext* const gfx_context, class Buffer* buffer, AccessFlags destination_access, PipelineStageType destination_stage)
		{ }

		void add_image_barrier(class GraphicsContext* const gfx_context, class Image* image, AccessFlags destination_access, PipelineStageType destination_stage, ImageLayout final_layout)
		{ }

		void execute(class GraphicsContext* const gfx_context, void* command_buffer, Identity execution_id = "")
		{ }

		void reset()
		{ }
	};

#if USE_VULKAN_GRAPHICS
	class BarrierBatcher : public GenericBarrierBatcher<VulkanBarrierBatcher>
	{ };
#else
	class BarrierBatcher : public GenericBarrierBatcher<NoopBarrierBatcher>
	{ };
#endif
}
