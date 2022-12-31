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

			GraphicsCommandQueue* graphics_command_queue() const
			{
				return command_queue.get();
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
			RenderTaskFrameAllocator rendertask_allocator;
			std::unique_ptr<GraphicsContext> graphics_context;
			class Swapchain* swapchain;
			std::unique_ptr<GraphicsCommandQueue> command_queue;
			class RenderPass* graphics_master_pass;
			Window* graphics_window;

			DescriptorData global_descriptor_data[MAX_BUFFERED_FRAMES];
			DescriptorData object_descriptor_data[MAX_BUFFERED_FRAMES];
	};
}
