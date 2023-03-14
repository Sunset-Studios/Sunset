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

				swapchain->request_next_image(graphics_context.get());

				task_allocator.reset();

				strategy.render(graphics_context.get(), render_graph, swapchain);

				swapchain->present(graphics_context.get(), DeviceQueueType::Graphics);

				graphics_context->advance_frame();
			}

			void draw_fullscreen_quad(void* command_buffer);

			GraphicsContext* context() const
			{
				return graphics_context.get();
			}

			inline RenderGraph& get_render_graph()
			{
				return render_graph;
			}

			inline MeshTaskQueue& get_mesh_task_queue()
			{
				return mesh_task_queue;
			}

			inline MeshRenderTask* fresh_rendertask()
			{
				return task_allocator.get_new();
			}

			inline DrawCullData& get_draw_cull_data()
			{
				return current_draw_cull_data;
			}

			inline void set_draw_cull_data(DrawCullData draw_cull_data)
			{
				current_draw_cull_data = draw_cull_data;
			}

			inline void wait_for_gpu()
			{
				graphics_context->wait_for_gpu();
			}

			void begin_frame();
			void queue_graph_command(Identity name, std::function<void(class RenderGraph&, RGFrameData&, void*)> command_callback);

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
			DrawCullData current_draw_cull_data;
	};

	// Handles renderer lifetime calls (render graph begin/submit, renderer draw, etc.)
	// as a scoped RAII object
	template<typename RenderStrategy>
	class ScopedRender
	{
	public:
		ScopedRender(Renderer* renderer)
			: renderer(renderer)
		{
			renderer->wait_for_gpu();
			renderer->begin_frame();
		}

		~ScopedRender()
		{
			renderer->draw<RenderStrategy>();
		}

	private:
		Renderer* renderer{ nullptr };
	};
}

#define QUEUE_RENDERGRAPH_COMMAND(CommandName, Lambda) Renderer::get()->queue_graph_command(#CommandName, Lambda)