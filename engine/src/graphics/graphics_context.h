#pragma once

#include <utility/execution_queue.h>

#include <common.h>

namespace Sunset
{
	template<class Policy>
	class GenericGraphicsContext
	{
		public:
			GenericGraphicsContext() = default;

			void initialize(class Window* const window)
			{
				graphics_policy.initialize(window);
			}

			void destroy()
			{
				graphics_policy.destroy(resource_deletion_queue);
			}

			void wait_for_gpu()
			{
				graphics_policy.wait_for_gpu();
			}

			void draw(void* buffer, uint32_t vertex_count, uint32_t instance_count, uint32_t instance_index = 0)
			{
				graphics_policy.draw(buffer, vertex_count, instance_count, instance_index);
			}

			void draw_indexed(void* buffer, uint32_t index_count, uint32_t instance_count, uint32_t instance_index = 0)
			{
				graphics_policy.draw_indexed(buffer, index_count, instance_count, instance_index);
			}

			void draw_indexed_indirect(void* buffer, class Buffer* indirect_buffer, uint32_t draw_count, uint32_t draw_first = 0)
			{
				graphics_policy.draw_indexed_indirect(buffer, indirect_buffer, draw_count, draw_first);
			}

			void dispatch_compute(void* buffer, uint32_t count_x, uint32_t count_y, uint32_t count_z)
			{
				graphics_policy.dispatch_compute(buffer, count_x, count_y, count_z);
			}

			void* get_state()
			{
				return graphics_policy.get_state();
			}

			class Window* get_window()
			{
				return graphics_policy.get_window();
			}

			uint32_t get_frame_number() const
			{
				return graphics_policy.get_frame_number();
			}

			uint16_t get_buffered_frame_number() const
			{
				return graphics_policy.get_buffered_frame_number();
			}

			void set_buffer_allocator(class BufferAllocator* allocator)
			{
				graphics_policy.set_buffer_allocator(allocator);
			}

			void set_descriptor_set_allocator(class DescriptorSetAllocator* allocator)
			{
				graphics_policy.set_descriptor_set_allocator(allocator);
			}

			class BufferAllocator* get_buffer_allocator()
			{
				return graphics_policy.get_buffer_allocator();
			}

			class DescriptorSetAllocator* get_descriptor_set_allocator()
			{
				return graphics_policy.get_descriptor_set_allocator();
			}

			void register_command_queue(DeviceQueueType queue_type)
			{
				graphics_policy.register_command_queue(queue_type);
			}

			void destroy_command_queue(DeviceQueueType queue_type)
			{
				graphics_policy.destroy_command_queue(queue_type);
			}

			class CommandQueue* get_command_queue(DeviceQueueType queue_type)
			{
				return graphics_policy.get_command_queue(queue_type);
			}

			void advance_frame()
			{
				graphics_policy.advance_frame();
			}

			void add_resource_deletion_execution(const std::function<void()>& execution)
			{
				resource_deletion_queue.push_execution(execution);
			}

			void push_constants(void* buffer, PipelineStateID pipeline_state, const PushConstantPipelineData& push_constant_data)
			{
				graphics_policy.push_constants(buffer, pipeline_state, push_constant_data);
			}

			void push_descriptor_writes(const std::vector<DescriptorWrite>& descriptor_writes)
			{
				graphics_policy.push_descriptor_writes(descriptor_writes);
			}

			size_t get_min_ubo_offset_alignment()
			{
				return graphics_policy.get_min_ubo_offset_alignment();
			}

			void update_indirect_draw_command(void* commands, uint32_t command_index, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint64_t object_id, uint32_t batch_id)
			{
				graphics_policy.update_indirect_draw_command(commands, command_index, index_count, first_index, instance_count, first_instance, object_id, batch_id);
			}

			ShaderLayoutID derive_layout_for_shader_stages(class GraphicsContext* const gfx_context, const std::vector<PipelineShaderStage>& stages)
			{
				return graphics_policy.derive_layout_for_shader_stages(gfx_context, stages);
			}

		private:
			Policy graphics_policy;
			ExecutionQueue resource_deletion_queue;
	};

	class NoopGraphicsContext
	{
		public:
			NoopGraphicsContext() = default;

			void initialize(class Window* const window)
			{ }

			void destroy(ExecutionQueue& deletion_queue)
			{ }

			void wait_for_gpu()
			{ }

			void draw(void* buffer, uint32_t vertex_count, uint32_t instance_count, uint32_t instance_index = 0)
			{ }

			void draw_indexed(void* buffer, uint32_t index_count, uint32_t instance_count, uint32_t instance_index = 0)
			{ }

			void draw_indexed_indirect(void* buffer, class Buffer* indirect_buffer, uint32_t draw_count, uint32_t draw_first = 0)
			{ }

			void dispatch_compute(void* buffer, uint32_t count_x, uint32_t count_y, uint32_t count_z)
			{ }

			class Window* get_window()
			{
				return nullptr;
			}

			void* get_state()
			{
				return nullptr;
			}

			void set_buffer_allocator(class BufferAllocator* allocator)
			{ }

			void set_descriptor_set_allocator(class DescriptorSetAllocator* allocator)
			{ }

			class BufferAllocator* get_buffer_allocator()
			{
				return nullptr;
			}

			class DescriptorSetAllocator* get_descriptor_set_allocator()
			{
				return nullptr;
			}

			void register_command_queue(DeviceQueueType queue_type)
			{ }

			void destroy_command_queue(DeviceQueueType queue_type)
			{ }

			class CommandQueue* get_command_queue(DeviceQueueType queue_type)
			{
				return nullptr;
			}

			uint32_t get_frame_number() const
			{
				return 0;
			}

			uint16_t get_buffered_frame_number() const
			{
				return 0;
			}

			void advance_frame()
			{ }

			void push_constants(void* buffer, PipelineStateID pipeline_state, const PushConstantPipelineData& push_constant_data)
			{ }

			void push_descriptor_writes(const std::vector<DescriptorWrite>& descriptor_writes)
			{ }

			size_t get_min_ubo_offset_alignment()
			{
				return 0;
			}

			void update_indirect_draw_command(void* commands, uint32_t command_index, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint64_t object_id, uint32_t batch_id)
			{ }

			ShaderLayoutID derive_layout_for_shader_stages(class GraphicsContext* const gfx_context, const std::vector<PipelineShaderStage>& stages)
			{ }
	};

#if USE_VULKAN_GRAPHICS
	class GraphicsContext : public GenericGraphicsContext<VulkanContext>
	{ };
#else
	class GraphicsContext : public GenericGraphicsContext<NoopGraphicsContext>
	{ };
#endif

	class GraphicsContextFactory
	{
	public:
		template<typename ...Args>
		static std::unique_ptr<GraphicsContext> create(Args&&... args)
		{
			GraphicsContext* gfx = new GraphicsContext;
			gfx->initialize(std::forward<Args>(args)...);
			return std::unique_ptr<GraphicsContext>(gfx);
		}
	};
}
