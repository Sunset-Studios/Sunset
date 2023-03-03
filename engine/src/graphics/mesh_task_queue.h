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
		void operator()(class GraphicsContext* const gfx_context, void* command_buffer, RenderPassID render_pass, DescriptorSet* pass_descriptor_set, MaterialID material, ResourceStateID resource_state, PipelineStateID pipeline_state, const PushConstantPipelineData& push_constants = {});
		void operator()(class GraphicsContext* const gfx_context, void* command_buffer, RenderPassID render_pass, DescriptorSet* pass_descriptor_set, const IndirectDrawBatch& indirect_draw, class Buffer* indirect_buffer, PipelineStateID pipeline_state, const PushConstantPipelineData& push_constants = {});

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
			MeshTaskQueue() = default;
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

			void sort_and_batch(class GraphicsContext* const gfx_context);
			void submit_compute_cull(class GraphicsContext* const gfx_context, void* command_buffer);
			void submit_draws(class GraphicsContext* const gfx_context, void* command_buffer, RenderPassID render_pass, DescriptorSet* pass_descriptor_set, PipelineStateID pipeline_state, bool b_flush = true);

		private:
			std::vector<IndirectDrawBatch> batch_indirect_draws(class GraphicsContext* const gfx_context);
			void update_indirect_draw_buffers(class GraphicsContext* const gfx_context, void* command_buffer);

		private:
			std::vector<class MeshRenderTask*> queue;
			std::vector<size_t> previous_queue_hashes;
			MeshRenderTaskExecutor draw_executor;
			MeshRenderTaskExecutor compute_cull_executor;
			IndirectDrawData indirect_draw_data;
			GPUDrawIndirectBuffers indirect_draw_buffers;
	};
}
