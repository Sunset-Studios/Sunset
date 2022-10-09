#pragma once

#include <common.h>
#include <graphics/command_queue.h>
#include <graphics/graphics_context.h>

namespace Sunset
{
	class Renderer
	{
		public:
			Renderer() = default;
			Renderer(Renderer&& other) = delete;
			Renderer(const Renderer& other) = delete;
			Renderer& operator=(const Renderer& other) = delete;
			~Renderer() = default;

		public:
			void initialize(class Window* const window);
			void draw();
			void destroy();

		protected:
			class BufferAllocator* buffer_allocator;
			std::unique_ptr<GraphicsContext> graphics_context;
			class Swapchain* swapchain;
			std::unique_ptr<GraphicsCommandQueue> command_queue;
			class RenderPass* render_pass;
	};

	class RendererFactory
	{
	public:
		template<typename ...Args>
		static Renderer* create(Args&&... args)
		{
			Renderer* renderer = new Renderer;
			renderer->initialize(std::forward<Args>(args)...);
			return renderer;
		}
	};
}
