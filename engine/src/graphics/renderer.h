#pragma once

#include <common.h>

namespace Sunset
{
	class Renderer
	{
		public:
			Renderer() = default;
			Renderer(Renderer&& other) = delete;
			Renderer(const Renderer& other) = delete;
			Renderer& operator=(const Renderer& other) = delete;

		public:
			void initialize(class Window* const window);
			void draw();
			void destroy();

		protected:
			class GraphicsContext* graphics_context;
			class Swapchain* swapchain;
			class GraphicsCommandQueue* command_queue;
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
