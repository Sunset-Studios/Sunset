#pragma once

#include <common.h>
#include <singleton.h>
#include <graphics/graphics_context.h>
#include <graphics/render_graph.h>
#include <graphics/task_queue.h>
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

			DescriptorData get_global_descriptor_data(uint16_t buffered_frame) const
			{
				return global_descriptor_data[buffered_frame];
			}

			DescriptorData get_object_descriptor_data(uint16_t buffered_frame) const
			{
				return object_descriptor_data[buffered_frame];
			}

			class DescriptorSet* global_descriptor_set(uint16_t buffered_frame) const
			{
				return global_descriptor_data[buffered_frame].descriptor_set;
			}

			class DescriptorSet* object_descriptor_set(uint16_t buffered_frame) const
			{
				return object_descriptor_data[buffered_frame].descriptor_set;
			}

			class DescriptorLayout* global_descriptor_layout(uint16_t buffered_frame) const
			{
				return global_descriptor_data[buffered_frame].descriptor_layout;
			}

			class DescriptorLayout* object_descriptor_layout(uint16_t buffered_frame) const
			{
				return object_descriptor_data[buffered_frame].descriptor_layout;
			}

			RenderGraph& get_render_graph();
			TaskQueue& get_mesh_task_queue();
			RenderTask* fresh_rendertask();

			void inject_global_descriptor(uint16_t buffered_frame, const std::initializer_list<DescriptorBuildData>& descriptor_build_datas);
			void inject_object_descriptor(uint16_t buffered_frame, const std::initializer_list<DescriptorBuildData>& descriptor_build_datas);

		private:
			Renderer() = default;
			Renderer(Renderer&& other) = delete;
			Renderer(const Renderer& other) = delete;
			Renderer& operator=(const Renderer& other) = delete;
			~Renderer() = default;

		protected:
			std::unique_ptr<GraphicsContext> graphics_context;
			class Swapchain* swapchain;

			// TODO: Rename class to something more akin to it's purpose since it's
			// mostly only used for mesh/object render tasks
			RenderTaskFrameAllocator task_allocator;
			TaskQueue mesh_task_queue;
			RenderGraph render_graph;

			DescriptorData global_descriptor_data[MAX_BUFFERED_FRAMES];
			DescriptorData object_descriptor_data[MAX_BUFFERED_FRAMES];
	};
}
