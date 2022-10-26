#pragma once

#include <common.h>
#include <singleton.h>
#include <graphics/command_queue.h>
#include <graphics/graphics_context.h>
#include <graphics/render_task.h>
#include <window/window.h>

namespace Sunset
{
	class Renderer : public Singleton<Renderer>
	{
		friend class Singleton;

		public:
			void initialize() { }
			void setup(class Window* const window);
			void draw();
			void destroy();

			GraphicsContext* context() const
			{
				return graphics_context.get();
			}

			Window* window() const
			{
				return graphics_window;
			}

			class RenderPass* master_pass() const
			{
				return graphics_master_pass;
			}

			RenderTask* fresh_rendertask();

		private:
			Renderer() = default;
			Renderer(Renderer&& other) = delete;
			Renderer(const Renderer& other) = delete;
			Renderer& operator=(const Renderer& other) = delete;
			~Renderer() = default;

		protected:
			class BufferAllocator* buffer_allocator;
			RenderTaskFrameAllocator rendertask_allocator;
			std::unique_ptr<GraphicsContext> graphics_context;
			class Swapchain* swapchain;
			std::unique_ptr<GraphicsCommandQueue> command_queue;
			class RenderPass* graphics_master_pass;
			Window* graphics_window;
	};
}
