#pragma once

#include <common.h>
#include <singleton.h>
#include <graphics/graphics_context.h>
#include <graphics/render_graph.h>

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

			RenderGraph render_graph;

			DescriptorData global_descriptor_data[MAX_BUFFERED_FRAMES];
			DescriptorData object_descriptor_data[MAX_BUFFERED_FRAMES];
	};
}
