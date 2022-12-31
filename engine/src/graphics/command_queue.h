#pragma once

#include <common.h>

namespace Sunset
{
	template<class Policy>
	class GenericCommandQueue
	{
	public:
		GenericCommandQueue() = default;

		void initialize(class GraphicsContext* const gfx_context)
		{
			queue_policy.initialize(gfx_context);
		}

		void destroy(class GraphicsContext* const gfx_context)
		{
			queue_policy.destroy(gfx_context);
		}

		void* get_data()
		{
			return queue_policy.get_data();
		}

		void new_command_pool(class GraphicsContext* const gfx_context, void* command_pool_ptr)
		{
			queue_policy.new_command_pool(gfx_context, command_pool_ptr);
		}

		void new_command_buffers(class GraphicsContext* const gfx_context, void* command_buffer_ptr, void* command_pool_ptr, uint16_t count = 1)
		{
			queue_policy.new_command_buffers(gfx_context, command_buffer_ptr, command_pool_ptr, count);
		}

		void* begin_one_time_buffer_record(class GraphicsContext* const gfx_context)
		{
			return queue_policy.begin_one_time_buffer_record(gfx_context);
		}

		void end_one_time_buffer_record(class GraphicsContext* const gfx_context)
		{
			queue_policy.end_one_time_buffer_record(gfx_context);
		}

		void submit(class GraphicsContext* const gfx_context)
		{
			queue_policy.submit(gfx_context);
		}

		void submit_immediate(class GraphicsContext* const gfx_context, const std::function<void(void* cmd_buffer)>& buffer_update_fn)
		{
			queue_policy.submit_immediate(gfx_context, buffer_update_fn);
		}

	private:
		Policy queue_policy;
	};

	class NoopCommandQueue
	{
	public:
		NoopCommandQueue() = default;

		void initialize(class GraphicsContext* const gfx_context)
		{ }

		void destroy(class GraphicsContext* const gfx_context)
		{ }

		void* get_data()
		{
			return nullptr;
		}

		void new_command_pool(class GraphicsContext* const gfx_context, void* command_pool_ptr)
		{
			command_pool_ptr = nullptr;
		}

		void new_command_buffers(class GraphicsContext* const gfx_context, void* command_buffer_ptr, void* command_pool_ptr, uint16_t count = 1)
		{
			command_buffer_ptr = nullptr;
		}

		void* begin_one_time_buffer_record(class GraphicsContext* const gfx_context)
		{
			return nullptr;
		}

		void end_one_time_buffer_record(class GraphicsContext* const gfx_context)
		{ }

		void submit(class GraphicsContext* const gfx_context)
		{ }

		void submit_immediate(class GraphicsContext* const gfx_context, const std::function<void(void* cmd_buffer)>& buffer_update_fn)
		{ }
	};

#if USE_VULKAN_GRAPHICS
	class GraphicsCommandQueue : public GenericCommandQueue<VulkanCommandQueue>
	{ };
#else
	class GraphicsCommandQueue : public GenericCommandQueue<NoopCommandQueue>
	{ };
#endif

	class GraphicsCommandQueueFactory
	{
	public:
		template<typename ...Args>
		static std::unique_ptr<GraphicsCommandQueue> create(Args&&... args)
		{
			GraphicsCommandQueue* gfx = new GraphicsCommandQueue;
			gfx->initialize(std::forward<Args>(args)...);
			return std::unique_ptr<GraphicsCommandQueue>(gfx);
		}
	};
}
