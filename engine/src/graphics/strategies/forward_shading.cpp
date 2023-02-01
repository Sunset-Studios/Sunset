#include <graphics/strategies/forward_shading.h>
#include <graphics/graphics_context.h>
#include <graphics/renderer.h>
#include <window/window.h>
#ifndef NDEBUG
#include <utility/gui/gui_core.h>
#endif

namespace Sunset
{
	void ForwardShadingStrategy::render(GraphicsContext* gfx_context, RenderGraph& render_graph, class Swapchain* swapchain)
	{
		render_graph.add_pass(
			gfx_context,
			"general_compute_pass",
			RenderPassFlags::Compute,
			{},
			[](RenderGraph& graph, void* command_buffer)
			{

			}
		);

		{
			const glm::vec2 image_extent = gfx_context->get_window()->get_extent();
			RGResourceHandle color_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "main_color",
					.format = swapchain->get_format(),
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Present,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::Repeat,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true,
					.has_store_op = true,
				}
			);
			RGResourceHandle depth_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "main_depth",
					.format = Format::FloatDepth32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::DepthStencil | ImageFlags::Image2D,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::Repeat,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true,
					.attachment_stencil_clear = true,
				}
			);

			render_graph.add_pass(
				gfx_context,
				"forward_pass",
				RenderPassFlags::Main,
				{
					.outputs = { color_image_desc, depth_image_desc }
				},
				[=](RenderGraph& graph, void* command_buffer)
				{
					Renderer::get()->get_mesh_task_queue().submit(gfx_context, command_buffer);
				}
			);

#ifndef NDEBUG
			render_graph.add_pass(
				gfx_context,
				"tools_gui",
				RenderPassFlags::Main,
				{},
				[=](RenderGraph& graph, void* command_buffer)
				{
					global_gui_core.initialize(gfx_context, gfx_context->get_window());
					global_gui_core.begin_draw();
					global_gui_core.end_draw(command_buffer);
				}
			);
#endif
		}

		render_graph.submit(gfx_context, swapchain);
	}
}
