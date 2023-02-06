#pragma once

#include <common.h>
#include <singleton.h>
#include <graphics/graphics_context.h>
#include <graphics/render_graph.h>
#include <graphics/mesh_task_queue.h>
#include <graphics/mesh_render_task.h>
#include <graphics/resource/swapchain.h>

namespace Sunset
{
	class Renderer : public Singleton<Renderer>
	{
		friend class Singleton;

		public:
			void initialize() { }
			void setup(class Window* const window);
			void destroy();

			template<typename RenderStrategy>
			void draw()
			{
				static RenderStrategy strategy;

				graphics_context->wait_for_gpu();

				swapchain->request_next_image(graphics_context.get());

				task_allocator.reset();

				strategy.render(graphics_context.get(), render_graph, swapchain);

				swapchain->present(graphics_context.get(), DeviceQueueType::Graphics);

				graphics_context->advance_frame();
			}

			GraphicsContext* context() const
			{
				return graphics_context.get();
			}

			RenderGraph& get_render_graph();
			MeshTaskQueue& get_mesh_task_queue();
			MeshRenderTask* fresh_rendertask();

		private:
			Renderer() = default;
			Renderer(Renderer&& other) = delete;
			Renderer(const Renderer& other) = delete;
			Renderer& operator=(const Renderer& other) = delete;
			~Renderer() = default;

		protected:
			std::unique_ptr<GraphicsContext> graphics_context;
			class Swapchain* swapchain;

			MeshRenderTaskFrameAllocator task_allocator;
			MeshTaskQueue mesh_task_queue;
			RenderGraph render_graph;
	};
}
