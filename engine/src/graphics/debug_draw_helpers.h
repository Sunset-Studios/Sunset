#pragma once

#include <minimal.h>
#include <graphics/pipeline_types.h>
#include <graphics/resource_types.h>
#include <utility/pattern/singleton.h>
#include <gpu_shared_data_types.h>
#include <memory/allocators/stack_allocator.h>

namespace Sunset
{
	constexpr uint32_t MIN_DEBUG_DRAW_PRIMITIVES = 65536;

	class DebugDrawExecutor
	{
	public:
		DebugDrawExecutor() = default;
		~DebugDrawExecutor() = default;

		void reset()
		{
			cached_resource_state = 0;
		}

		void operator()(
			class GraphicsContext* const gfx_context,
			void* command_buffer,
			ResourceStateID resource_state,
			PipelineStateID pipeline_state,
			int32_t buffered_frame_number,
			uint32_t instance_count = 1,
			const PushConstantPipelineData& push_constants = {}
		);

	private:
		ResourceStateID cached_resource_state{ 0 };
	};

	struct DebugPrimitiveData
	{
		glm::mat4 transform;
		glm::vec4 color;
	};

	DECLARE_GPU_SHARED_DATA(DebugPrimitiveData, MIN_DEBUG_DRAW_PRIMITIVES);

	class DebugDrawState : public Singleton<DebugDrawState>
	{
		friend class Singleton;
		
	public:
		void initialize();

	public:
		BufferID primitive_data_buffers[MAX_BUFFERED_FRAMES];
		StaticFrameAllocator<DebugPrimitiveData, MIN_DEBUG_DRAW_PRIMITIVES> requested_primitive_datas[MAX_BUFFERED_FRAMES];
	};

	void debug_draw_line(class GraphicsContext* const gfx_context, glm::vec3 line_start, glm::vec3 line_end, glm::vec3 line_color, int32_t buffered_frame_number);

	void submit_requested_debug_draws_auto_shader(class GraphicsContext* const gfx_context, void* command_buffer, RenderPassID render_pass, int32_t buffered_frame_number);
	void submit_requested_debug_draws(class GraphicsContext* const gfx_context, void* command_buffer, PipelineStateID pipeline_state, int32_t buffered_frame_number);
}
