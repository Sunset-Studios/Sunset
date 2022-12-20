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

			void draw(void* buffer, uint32_t vertex_count, uint32_t instance_count)
			{
				graphics_policy.draw(buffer, vertex_count, instance_count);
			}

			void* get_state()
			{
				return graphics_policy.get_state();
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

			void draw(void* buffer, uint32_t vertex_count, uint32_t instance_count)
			{ }

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
