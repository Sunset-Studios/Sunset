#pragma once

#include <utility>

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
				graphics_policy.destroy();
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

			void set_buffer_allocator(class BufferAllocator* allocator)
			{
				graphics_policy.set_buffer_allocator(allocator);
			}

			class BufferAllocator* get_buffer_allocator()
			{
				return graphics_policy.get_buffer_allocator();
			}

			void advance_frame()
			{
				graphics_policy.advance_frame();
			}

		private:
			Policy graphics_policy;
	};

	class NoopGraphicsContext
	{
		public:
			NoopGraphicsContext() = default;

			void initialize(class Window* const window)
			{ }

			void destroy()
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

			class BufferAllocator* get_buffer_allocator()
			{
				return nullptr;
			}

			uint32_t get_frame_number() const
			{
				return 0;
			}

			void advance_frame()
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
		static GraphicsContext* create(Args&&... args)
		{
			GraphicsContext* gfx = new GraphicsContext;
			gfx->initialize(std::forward<Args>(args)...);
			return gfx;
		}
	};
}
