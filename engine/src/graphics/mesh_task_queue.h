#pragma once

#include <minimal.h>
#include <graphics/resource/material.h>
#include <graphics/renderer_types.h>

namespace Sunset
{
	class MeshRenderTaskExecutor
	{
	public:
		MeshRenderTaskExecutor() = default;
		~MeshRenderTaskExecutor() = default;

		void reset();
		void operator()(class GraphicsContext* const gfx_context, void* command_buffer, RenderPassID render_pass, ResourceStateID resource_state, PipelineStateID pipeline_state, int32_t buffered_frame_number, uint32_t instance_count = 1, const PushConstantPipelineData& push_constants = {});
		void operator()(class GraphicsContext* const gfx_context, void* command_buffer, RenderPassID render_pass, DescriptorSet* descriptor_set, const IndirectDrawBatch& indirect_draw, uint32_t indirect_draw_index, class Buffer* indirect_buffer, PipelineStateID pipeline_state, int32_t buffered_frame_number, const PushConstantPipelineData& push_constants = {});

	private:
		MaterialID cached_material{ 0 };
		ResourceStateID cached_resource_state{ 0 };
	};

	class MeshComputeCullTaskExecutor
	{
	public:
		MeshComputeCullTaskExecutor() = default;
		~MeshComputeCullTaskExecutor() = default;

		void operator()(class GraphicsContext* const gfx_context, void* command_buffer, RenderPassID render_pass, PipelineStateID pipeline_state, const PushConstantPipelineData& push_constants = {}, const std::vector<DescriptorLayoutID>& descriptor_layouts = {});
	};

	class MeshTaskQueue
	{
		public:
			MeshTaskQueue()
			{
				queue.reserve(MIN_ENTITIES);
			}
			~MeshTaskQueue() = default;

			bool empty() const
			{
				return queue.empty();
			}

			void add(class MeshRenderTask* const task)
			{
				queue.emplace_back(task);
			}

			void set_gpu_draw_indirect_buffers(const GPUDrawIndirectBuffers& buffers)
			{
				indirect_draw_buffers = buffers;
			}

			void set_gpu_draw_indirect_data(const IndirectDrawData& data)
			{
				indirect_draw_data = data;
			}

			void set_is_deferred_rendering(bool b_is_deferred = true)
			{
				b_is_deferred_rendering = b_is_deferred;
			}

			GPUDrawIndirectBuffers& get_gpu_draw_indirect_buffers()
			{
				return indirect_draw_buffers;
			}

			IndirectDrawData& get_indirect_draw_data()
			{
				return indirect_draw_data;
			}

			uint32_t get_queue_size() const
			{
				return queue.size();
			}

			uint32_t get_num_indirect_batches() const
			{
				return indirect_draw_data.indirect_draws.size();
			}

			bool is_deferred_rendering() const
			{
				return b_is_deferred_rendering;
			}

			void sort_and_batch(class GraphicsContext* const gfx_context);
			void submit_compute_cull(class GraphicsContext* const gfx_context, void* command_buffer, int32_t buffered_frame_number, ExecutionQueue* deletion_queue = nullptr);
			void submit_draws(class GraphicsContext* const gfx_context, void* command_buffer, RenderPassID render_pass, DescriptorSet* descriptor_set, PipelineStateID pipeline_state, int32_t buffered_frame_number, bool b_use_draw_push_constants = true, bool b_flush = true);
			void submit_bounds_debug_draws(class GraphicsContext* const gfx_context, void* command_buffer, RenderPassID render_pass, int32_t buffered_frame_number);

		private:
			std::vector<IndirectDrawBatch> batch_indirect_draws(class GraphicsContext* const gfx_context);
			void update_indirect_draw_buffers(class GraphicsContext* const gfx_context, void* command_buffer, int32_t buffered_frame_number, ExecutionQueue* deletion_queue = nullptr);

		private:
			std::vector<class MeshRenderTask*> queue;
			MeshRenderTaskExecutor draw_executor;
			MeshRenderTaskExecutor compute_cull_executor;
			IndirectDrawData indirect_draw_data{};
			GPUDrawIndirectBuffers indirect_draw_buffers{};
			bool b_is_deferred_rendering{ false };
	};
}
