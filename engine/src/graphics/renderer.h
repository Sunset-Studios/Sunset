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
			void setup(class Window* const window, const glm::ivec2& resolution = glm::ivec2(800, 800)); 
			void destroy();

			template<typename RenderStrategy>
			Sunset::ThreadedJob<1> draw(bool b_offline = false, int32_t buffered_frame_to_render = -1)
			{
				ZoneScopedN("Renderer::draw");

				static RenderStrategy strategy;

				const uint32_t buffered_frame = buffered_frame_to_render >= 0 ? buffered_frame_to_render : graphics_context->get_buffered_frame_number();

				if (!b_offline && b_last_work_submitted)
				{
					swapchain->request_next_image(graphics_context.get(), buffered_frame);
				}

				b_last_work_submitted = strategy.render(graphics_context.get(), render_graph, swapchain, buffered_frame, b_offline);

				if (!b_offline && b_last_work_submitted)
				{
					swapchain->present(graphics_context.get(), DeviceQueueType::Graphics, buffered_frame);
				}

				building_command_list[buffered_frame].store(false);

				co_return;
			}

			void draw_fullscreen_quad(void* command_buffer);
			void draw_unit_sphere(void* command_buffer);
			void draw_unit_cube(void* command_buffer);

			GraphicsContext* context() const
			{
				return graphics_context.get();
			}

			inline RenderGraph& get_render_graph()
			{
				return render_graph;
			}

			inline MeshTaskQueue& get_mesh_task_queue(uint32_t buffered_frame_number)
			{
				return mesh_task_queue[buffered_frame_number];
			}

			inline MeshRenderTask* fresh_rendertask()
			{
				return task_allocator[graphics_context->get_buffered_frame_number()].get_new();
			}

			inline DrawCullData& get_draw_cull_data(int32_t buffered_frame_number)
			{
				return current_draw_cull_data[buffered_frame_number];
			}

			inline void set_draw_cull_data(DrawCullData draw_cull_data, int32_t buffered_frame_number)
			{
				current_draw_cull_data[buffered_frame_number] = draw_cull_data;
			}

			inline void wait_for_gpu()
			{
				ZoneScopedN("Renderer::wait_for_gpu");
				graphics_context->wait_for_gpu();
			}

			void set_is_building_command_list(uint32_t buffered_frame_number, bool b_building)
			{
				building_command_list[buffered_frame_number].store(b_building);
			}

			void wait_for_command_list_build();
			void begin_frame();
			void queue_graph_command(Identity name, std::function<void(class RenderGraph&, RGFrameData&, void*)> command_callback);
			void register_persistent_image(Identity id, ImageID image);
			ImageID get_persistent_image(Identity id, uint32_t buffered_frame = 0);

		private:
			Renderer() = default;
			Renderer(Renderer&& other) = delete;
			Renderer(const Renderer& other) = delete;
			Renderer& operator=(const Renderer& other) = delete;
			~Renderer() = default;

		protected:
			std::unique_ptr<GraphicsContext> graphics_context;
			class Swapchain* swapchain;

			MeshRenderTaskFrameAllocator task_allocator[MAX_BUFFERED_FRAMES];
			MeshTaskQueue mesh_task_queue[MAX_BUFFERED_FRAMES];
			DrawCullData current_draw_cull_data[MAX_BUFFERED_FRAMES];
			RenderGraph render_graph;
			phmap::parallel_flat_hash_map<Identity, ImageID> persistent_image_map;

			bool b_last_work_submitted{ true };
			std::atomic_bool building_command_list[MAX_BUFFERED_FRAMES];
	};

	// Handles renderer lifetime calls (render graph begin/submit, renderer draw, etc.)
	// as a scoped RAII object
	template<typename RenderStrategy>
	class ScopedRender
	{
	public:
		ScopedRender(Renderer* renderer, bool b_offline = false)
			: renderer(renderer), b_offline(b_offline)
		{
			ZoneScopedN("ScopedRender::ScopedRender");
			renderer->begin_frame();
			buffered_frame_to_render = renderer->context()->get_buffered_frame_number();
		}

		~ScopedRender()
		{
			renderer->set_is_building_command_list(buffered_frame_to_render, true);
			renderer->draw<RenderStrategy>(b_offline, buffered_frame_to_render);
		}

	private:
		Renderer* renderer{ nullptr };
		bool b_offline{ false };
		int32_t buffered_frame_to_render{ 0 };
	};
}

#define QUEUE_RENDERGRAPH_COMMAND(CommandName, Lambda) Renderer::get()->queue_graph_command(#CommandName, Lambda)