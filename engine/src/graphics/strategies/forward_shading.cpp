#include <graphics/strategies/forward_shading.h>
#include <graphics/graphics_context.h>
#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <graphics/descriptor.h>
#include <graphics/resource/image.h>
#include <window/window.h>
#include <core/data_globals.h>
#ifndef NDEBUG
#include <utility/gui/gui_core.h>
#endif

namespace Sunset
{
	void ForwardShadingStrategy::render(GraphicsContext* gfx_context, RenderGraph& render_graph, class Swapchain* swapchain)
	{
		// Compute mesh cull pass
		{
			// Shader layout setup
			RGShaderDataSetup shader_setup
			{
				.declarations =
				{
					RGShaderDescriptorDeclaration{
						.count = 1,
						.type = DescriptorType::StorageBuffer,
						.shader_stages = PipelineShaderStageType::Compute
					},
					RGShaderDescriptorDeclaration{
						.count = 1,
						.type = DescriptorType::StorageBuffer,
						.shader_stages = PipelineShaderStageType::Compute
					},
					RGShaderDescriptorDeclaration{
						.count = 1,
						.type = DescriptorType::StorageBuffer,
						.shader_stages = PipelineShaderStageType::Compute
					},
					RGShaderDescriptorDeclaration{
						.count = 1,
						.type = DescriptorType::StorageBuffer,
						.shader_stages = PipelineShaderStageType::Compute
					},
					RGShaderDescriptorDeclaration{
						.count = 1,
						.type = DescriptorType::StorageBuffer,
						.shader_stages = PipelineShaderStageType::Compute
					},
				},
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Compute, "../../shaders/basic.comp.spv" }
				},
				.push_constant_data = PushConstantPipelineData::create(&Renderer::get()->get_draw_cull_data())
			};

			// Input resources
			RGResourceHandle entity_data_buffer_desc = render_graph.register_buffer(
				gfx_context,
				EntityGlobals::get()->entity_data.data_buffer[gfx_context->get_buffered_frame_number()]
			);

			RGResourceHandle object_instance_buffer_desc = render_graph.create_buffer(
				gfx_context,
				{
					.name = "object_instance_buffer",
					.buffer_size = 256,
					.type = BufferType::TransferDestination | BufferType::StorageBuffer,
					.memory_usage = MemoryUsageType::OnlyGPU
				}
			);

			RGResourceHandle compacted_object_instance_buffer_desc = render_graph.create_buffer(
				gfx_context,
				{
					.name = "compacted_object_instance_buffer",
					.buffer_size = 256,
					.type = BufferType::TransferDestination | BufferType::StorageBuffer,
					.memory_usage = MemoryUsageType::OnlyGPU
				}
			);

			RGResourceHandle cleared_draw_indirect_buffer_desc = render_graph.create_buffer(
				gfx_context,
				{
					.name = "cleared_draw_indirect_buffer",
					.buffer_size = 1,
					.type = BufferType::TransferSource | BufferType::StorageBuffer | BufferType::Indirect,
					.memory_usage = MemoryUsageType::CPUToGPU
				}
			);

			// Output resources
			RGResourceHandle draw_indirect_buffer_desc = render_graph.create_buffer(
				gfx_context,
				{
					.name = "draw_indirect_buffer",
					.buffer_size = 256,
					.type = BufferType::TransferDestination | BufferType::StorageBuffer | BufferType::Indirect,
					.memory_usage = MemoryUsageType::OnlyGPU
				}
			);

			render_graph.add_pass(
				gfx_context,
				"compute_cull",
				RenderPassFlags::Compute,
				{
					.shader_setup = shader_setup,
					.inputs = { entity_data_buffer_desc, object_instance_buffer_desc, compacted_object_instance_buffer_desc, cleared_draw_indirect_buffer_desc, draw_indirect_buffer_desc },
					.outputs = { draw_indirect_buffer_desc }
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					Renderer::get()->get_mesh_task_queue().set_gpu_draw_indirect_data(
						{
							.cleared_draw_indirect_buffer = static_cast<BufferID>(graph.get_physical_resource(cleared_draw_indirect_buffer_desc)),
							.draw_indirect_buffer = static_cast<BufferID>(graph.get_physical_resource(draw_indirect_buffer_desc)),
							.object_instance_buffer = static_cast<BufferID>(graph.get_physical_resource(object_instance_buffer_desc)),
							.compacted_object_instance_buffer = static_cast<BufferID>(graph.get_physical_resource(compacted_object_instance_buffer_desc))
						}
					);
					Renderer::get()->get_mesh_task_queue().submit_compute_cull(
						gfx_context,
						command_buffer
					);
				}
			);
		}

		// Forward pass
		RGResourceHandle main_color_image_desc;
		RGResourceHandle main_depth_image_desc;
		{
			// Shader layout setup
			RGShaderDataSetup shader_setup
			{
				.declarations =
				{
					RGShaderDescriptorDeclaration{
						.count = 1,
						.type = DescriptorType::StorageBuffer,
						.shader_stages = PipelineShaderStageType::All
					},
					RGShaderDescriptorDeclaration{
						.count = MAX_DESCRIPTOR_BINDINGS,
						.type = DescriptorType::Image,
						.shader_stages = PipelineShaderStageType::Fragment,
						.b_supports_bindless = true
					},
					RGShaderDescriptorDeclaration{
						.count = 1,
						.type = DescriptorType::StorageBuffer,
						.shader_stages = PipelineShaderStageType::Fragment,
					}
				}
			};

			// Input resources
			RGResourceHandle entity_data_buffer_desc = render_graph.register_buffer(
				gfx_context,
				EntityGlobals::get()->entity_data.data_buffer[gfx_context->get_buffered_frame_number()]
			);

			RGResourceHandle default_image_desc = render_graph.register_image(
				gfx_context,
				ImageFactory::create_default(gfx_context)
			);

			RGResourceHandle material_data_buffer_desc = render_graph.register_buffer(
				gfx_context,
				MaterialGlobals::get()->material_data.data_buffer[gfx_context->get_buffered_frame_number()]
			);

			// Output resources
			const glm::vec2 image_extent = gfx_context->get_window()->get_extent();
			main_color_image_desc = render_graph.create_image(
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
			main_depth_image_desc = render_graph.create_image(
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

			// Pass registration
			render_graph.add_pass(
				gfx_context,
				"forward_pass",
				RenderPassFlags::Main,
				{
					.shader_setup = shader_setup,
					.inputs = { entity_data_buffer_desc, default_image_desc, material_data_buffer_desc },
					.outputs = { main_color_image_desc, main_depth_image_desc }
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					Renderer::get()->get_mesh_task_queue().submit_draws(
						gfx_context,
						command_buffer,
						frame_data.current_pass
					);
				}
			);

		}

		// IMGui pass
#ifndef NDEBUG
		render_graph.add_pass(
			gfx_context,
			"tools_gui",
			RenderPassFlags::Main,
			{
				.outputs = { main_color_image_desc, main_depth_image_desc }
			},
			[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
			{
				global_gui_core.initialize(gfx_context, gfx_context->get_window(), frame_data.current_pass);
				global_gui_core.begin_draw();
				global_gui_core.end_draw(command_buffer);
			}
		);
#endif
		render_graph.submit(gfx_context, swapchain);
	}
}
