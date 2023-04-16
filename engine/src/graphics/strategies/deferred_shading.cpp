#include <graphics/strategies/deferred_shading.h>
#include <graphics/graphics_context.h>
#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <graphics/descriptor.h>
#include <graphics/resource/image.h>
#include <window/window.h>
#include <core/data_globals.h>
#include <utility/maths.h>
#include <utility/cvar.h>
#ifndef NDEBUG
#include <utility/gui/gui_core.h>
#endif

namespace Sunset
{
	AutoCVar_Int cvar_num_bloom_pass_iterations("ren.num_bloom_pass_iterations", "The number of bloom horizontal and vertical blur iterations. (0 to turn bloom off).", 5);
	AutoCVar_Float cvar_bloom_intensity("ren.bloom_intensity", "The intensity of the applied final bloom", 0.2f);

	AutoCVar_Float cvar_final_image_exposure("ren.final_image_exposure", "The exposure to apply once HDR color gets resolved down to LDR", 1.0f);

	void DeferredShadingStrategy::render(GraphicsContext* gfx_context, RenderGraph& render_graph, class Swapchain* swapchain)
	{
		const uint16_t buffered_frame_number = gfx_context->get_buffered_frame_number();

		MeshTaskQueue& mesh_task_queue = Renderer::get()->get_mesh_task_queue();
		mesh_task_queue.set_is_deferred_rendering(true);
		mesh_task_queue.sort_and_batch(gfx_context);

		RGResourceHandle object_instance_buffer_desc = render_graph.create_buffer(
			gfx_context,
			{
				.name = "object_instance_buffer",
				.buffer_size = mesh_task_queue.get_queue_size() * sizeof(GPUObjectInstance),
				.type = BufferType::TransferDestination | BufferType::StorageBuffer,
				.memory_usage = MemoryUsageType::OnlyGPU
			}
		);

		RGResourceHandle compacted_object_instance_buffer_desc = render_graph.create_buffer(
			gfx_context,
			{
				.name = "compacted_object_instance_buffer",
				.buffer_size = mesh_task_queue.get_queue_size() * sizeof(uint32_t),
				.type = BufferType::TransferDestination | BufferType::StorageBuffer,
				.memory_usage = MemoryUsageType::OnlyGPU
			}
		);

		RGResourceHandle draw_indirect_buffer_desc = render_graph.create_buffer(
			gfx_context,
			{
				.name = "draw_indirect_buffer",
				.buffer_size = mesh_task_queue.get_num_indirect_batches(),
				.type = BufferType::StorageBuffer | BufferType::Indirect,
				.memory_usage = MemoryUsageType::CPUToGPU
			}
		);

		RGResourceHandle entity_data_buffer_desc = render_graph.register_buffer(
			gfx_context,
			EntityGlobals::get()->entity_data.data_buffer[buffered_frame_number]
		);

		RGResourceHandle material_data_buffer_desc = render_graph.register_buffer(
			gfx_context,
			MaterialGlobals::get()->material_data.data_buffer[buffered_frame_number]
		);

		RGResourceHandle light_data_buffer_desc = render_graph.register_buffer(
			gfx_context,
			LightGlobals::get()->light_data.data_buffer[buffered_frame_number]
		);

		RGResourceHandle hzb_image_desc = render_graph.register_image(
			gfx_context,
			Renderer::get()->get_persistent_image("hi_z", buffered_frame_number)
		);

		// Compute mesh cull pass
		{
			render_graph.add_pass(
				gfx_context,
				"set_draw_cull_data_draw_count",
				RenderPassFlags::GraphLocal,
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					MeshTaskQueue& mesh_task_queue = Renderer::get()->get_mesh_task_queue();
					Renderer::get()->get_draw_cull_data().draw_count = mesh_task_queue.get_queue_size();
				}
			);

			RGShaderDataSetup shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Compute, "../../shaders/common/cull.comp.sun" }
				}
			};

			RGResourceHandle entity_data_buffer_desc = render_graph.register_buffer(
				gfx_context,
				EntityGlobals::get()->entity_data.data_buffer[buffered_frame_number]
			);

			render_graph.add_pass(
				gfx_context,
				"compute_cull",
				RenderPassFlags::Compute,
				{
					.shader_setup = shader_setup,
					.inputs = { entity_data_buffer_desc, object_instance_buffer_desc,
								compacted_object_instance_buffer_desc, draw_indirect_buffer_desc,
								hzb_image_desc },
					.outputs = { draw_indirect_buffer_desc }
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					{
						Image* const hzb_image = CACHE_FETCH(Image, graph.get_physical_resource(hzb_image_desc));
						DrawCullData& draw_cull_data = Renderer::get()->get_draw_cull_data();
						draw_cull_data.hzb_width = hzb_image->get_attachment_config().extent.x;
						draw_cull_data.hzb_height = hzb_image->get_attachment_config().extent.y;
						draw_cull_data.hzb_texture = frame_data.pass_bindless_resources.handles.front();
					}

					PushConstantPipelineData pass_data = PushConstantPipelineData::create(&Renderer::get()->get_draw_cull_data(), PipelineShaderStageType::Compute);

					gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pass_data);

					Renderer::get()->get_mesh_task_queue().set_gpu_draw_indirect_buffers(
						{
							.draw_indirect_buffer = static_cast<BufferID>(graph.get_physical_resource(draw_indirect_buffer_desc)),
							.object_instance_buffer = static_cast<BufferID>(graph.get_physical_resource(object_instance_buffer_desc)),
							.compacted_object_instance_buffer = static_cast<BufferID>(graph.get_physical_resource(compacted_object_instance_buffer_desc))
						}
					);
					Renderer::get()->get_mesh_task_queue().submit_compute_cull(
						gfx_context,
						command_buffer,
						frame_data.resource_deletion_queue
					);
				}
			);
		}

		// G-Buffer pass
		RGResourceHandle main_albedo_image_desc;
		RGResourceHandle main_depth_image_desc;
		RGResourceHandle main_specular_image_desc;
		RGResourceHandle main_normal_image_desc;
		RGResourceHandle main_position_image_desc;
		{
			RGShaderDataSetup shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Vertex, "../../shaders/deferred/deferred_mesh.vert.sun" },
					{ PipelineShaderStageType::Fragment, "../../shaders/deferred/deferred_gbuffer_base.frag.sun"}
				}
			};

			// Create G-Buffer targets
			const glm::vec2 image_extent = gfx_context->get_window()->get_extent();
			main_albedo_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "main_albedo",
					.format = Format::Float4x32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.clear_color = glm::vec3(0.2f, 0.9f, 0.5f),
					.flags = ImageFlags::Color | ImageFlags::Image2D | ImageFlags::Sampled,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::Repeat,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true,
					.attachment_stencil_clear = false
				}
			);
			main_depth_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "main_depth",
					.format = Format::FloatDepth32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::DepthStencil | ImageFlags::Sampled | ImageFlags::Image2D,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::EdgeClamp,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true,
					.attachment_stencil_clear = true,
				}
			);
			main_specular_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "main_specular",
					.format = Format::Float16,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Sampled | ImageFlags::Image2D,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::EdgeClamp,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true,
				}
			);
			main_normal_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "main_normal",
					.format = Format::Float4x32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Sampled | ImageFlags::Image2D,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::EdgeClamp,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true,
				}
			);
			main_position_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "main_position",
					.format = Format::Float4x32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Sampled | ImageFlags::Image2D,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::EdgeClamp,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true,
				}
			);

			render_graph.add_pass(
				gfx_context,
				"gbuffer_base_pass",
				RenderPassFlags::Graphics,
				{
					.shader_setup = shader_setup,
					.inputs = { entity_data_buffer_desc, material_data_buffer_desc,
								compacted_object_instance_buffer_desc,
								draw_indirect_buffer_desc },
					.outputs = { main_albedo_image_desc, main_depth_image_desc, main_specular_image_desc,
								 main_normal_image_desc, main_position_image_desc }
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					Renderer::get()->get_mesh_task_queue().submit_draws(
						gfx_context,
						command_buffer,
						frame_data.current_pass,
						frame_data.global_descriptor_set,
						frame_data.pass_pipeline_state
					);
				}
			);
		}

		// HZB generation pass
		{
			RGShaderDataSetup shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Compute, "../../shaders/common/hzb_reduce.comp.sun" }
				}
			};

			render_graph.add_pass(
				gfx_context,
				"reduce_hzb",
				RenderPassFlags::Compute,
				{
					.shader_setup = shader_setup,
					.inputs = { main_depth_image_desc, hzb_image_desc },
					.outputs = { hzb_image_desc },
					.b_split_input_image_mips = true
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					Image* const depth_image = CACHE_FETCH(Image, graph.get_physical_resource(main_depth_image_desc));
					Image* const hzb_image = CACHE_FETCH(Image, graph.get_physical_resource(hzb_image_desc));

					depth_image->barrier(
						gfx_context,
						command_buffer,
						depth_image->get_access_flags(),
						AccessFlags::ShaderRead,
						depth_image->get_layout(),
						ImageLayout::ShaderReadOnly,
						PipelineStageType::AllGraphics,
						PipelineStageType::ComputeShader
					);

					const uint32_t num_mips = hzb_image->get_num_image_views();
					for (uint32_t i = 0; i < num_mips; ++i)
					{
						const BindingTableHandle src_image_index = frame_data.pass_bindless_resources.handles[i == 0 ? 0 : 1 + (i - 1)];
						const BindingTableHandle dst_image_index = frame_data.pass_bindless_resources.handles[1 + num_mips + i];

						glm::ivec3 image_size = hzb_image->get_attachment_config().extent;
						const uint32_t mip_width = glm::clamp(image_size.x >> i, 1, image_size.x);
						const uint32_t mip_height = glm::clamp(image_size.y >> i, 1, image_size.y);

						{
							DepthReduceData reduce_data
							{
								.reduced_image_size = glm::vec2(mip_width, mip_height),
								.input_depth_index = (0x0000ffff & src_image_index),
								.output_depth_index = (0x0000ffff & dst_image_index)
							};
							PushConstantPipelineData pc_data = PushConstantPipelineData::create(&reduce_data, PipelineShaderStageType::Compute);
							gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pc_data);
						}

						gfx_context->dispatch_compute(
							command_buffer,
							static_cast<uint32_t>((mip_width + 31) / 32),
							static_cast<uint32_t>((mip_height + 31) / 32),
							1
						);

						hzb_image->barrier(
							gfx_context,
							command_buffer,
							AccessFlags::ShaderWrite,
							AccessFlags::ShaderRead,
							i == 0 ? ImageLayout::Undefined : ImageLayout::General,
							ImageLayout::General,
							PipelineStageType::ComputeShader,
							PipelineStageType::ComputeShader
						);
					}
				}
			);
		}

		// Lighting pass
		RGResourceHandle scene_color_desc;
		{
			RGShaderDataSetup shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Vertex, "../../shaders/common/fullscreen.vert.sun" },
					{ PipelineShaderStageType::Fragment, "../../shaders/deferred/deferred_lit.frag.sun"}
				}
			};

			const glm::vec2 image_extent = gfx_context->get_window()->get_extent();
			scene_color_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "scene_color",
					.format = Format::Float4x32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Image2D | ImageFlags::Sampled,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::Repeat,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true
				}
			);

			render_graph.add_pass(
				gfx_context,
				"lighting_pass",
				RenderPassFlags::Graphics,
				{
					.shader_setup = shader_setup,
					.inputs = { entity_data_buffer_desc, light_data_buffer_desc,
								compacted_object_instance_buffer_desc, main_albedo_image_desc,
								main_specular_image_desc, main_normal_image_desc, main_position_image_desc },
					.outputs = { scene_color_desc }
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					const BindingTableHandle albedo_image_handle = frame_data.pass_bindless_resources.handles[0];
					const BindingTableHandle specular_image_handle = frame_data.pass_bindless_resources.handles[1];
					const BindingTableHandle normal_image_handle = frame_data.pass_bindless_resources.handles[2];
					const BindingTableHandle position_image_handle = frame_data.pass_bindless_resources.handles[3];

					LightingPassData lighting_data
					{
						.albedo_texure = 0x0000ffff & albedo_image_handle,
						.specular_texure = 0x0000ffff & specular_image_handle,
						.normal_texure = 0x0000ffff & normal_image_handle,
						.position_texure = 0x0000ffff & position_image_handle
					};

					PushConstantPipelineData pass_data = PushConstantPipelineData::create(&lighting_data, PipelineShaderStageType::Fragment);

					gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pass_data);

					Renderer::get()->draw_fullscreen_quad(command_buffer);
				}
			);
		}

		RGResourceHandle final_color_desc = scene_color_desc;

		// Bloom pass
		const uint32_t num_iterations = cvar_num_bloom_pass_iterations.get();
		if (num_iterations > 0) {
			RGResourceHandle bloom_blur_image_desc;

			RGShaderDataSetup bloom_downsample_shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Compute, "../../shaders/effects/bloom_downsample.comp.sun" }
				}
			};

			RGShaderDataSetup bloom_upsample_shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Compute, "../../shaders/effects/bloom_upsample.comp.sun" }
				}
			};

			RGShaderDataSetup bloom_resolve_shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Vertex, "../../shaders/common/fullscreen.vert.sun" },
					{ PipelineShaderStageType::Fragment, "../../shaders/effects/bloom_resolve.frag.sun" }
				}
			};

			const glm::vec2 image_extent = gfx_context->get_window()->get_extent();
			const float extent_x = Maths::ppot(image_extent.x);
			const float extent_y = Maths::ppot(image_extent.y);

			bloom_blur_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "bloom_blur",
					.format = Format::Float4x32,
					.extent = glm::vec3(extent_x, extent_y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Image2D | ImageFlags::Sampled | ImageFlags::Storage,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::EdgeClamp,
					.image_filter = ImageFilter::Linear,
					.mip_count = num_iterations,
					.attachment_clear = true
				}
			);

			render_graph.add_pass(
				gfx_context,
				"bloom_downsample_pass",
				RenderPassFlags::Compute,
				{
					.shader_setup = bloom_downsample_shader_setup,
					.inputs = { scene_color_desc, bloom_blur_image_desc },
					.outputs = { bloom_blur_image_desc },
					.b_split_input_image_mips = true
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					Image* const color_image = CACHE_FETCH(Image, graph.get_physical_resource(scene_color_desc));

					color_image->barrier(
						gfx_context,
						command_buffer,
						color_image->get_access_flags(),
						AccessFlags::ShaderRead,
						color_image->get_layout(),
						ImageLayout::ShaderReadOnly,
						PipelineStageType::AllGraphics,
						PipelineStageType::ComputeShader
					);

					Image* const downsample_image = CACHE_FETCH(Image, graph.get_physical_resource(bloom_blur_image_desc));
					const uint32_t num_mips = downsample_image->get_num_image_views();

					for (uint32_t i = 0; i < num_mips; ++i)
					{
						const uint32_t src_index = i == 0 ? 0 : i - 1;
						const uint32_t dst_index = i;

						const BindingTableHandle src_image_index = frame_data.pass_bindless_resources.handles[i == 0 ? 0 : 1 + src_index];
						const BindingTableHandle dst_image_index = frame_data.pass_bindless_resources.handles[1 + num_mips + dst_index];

						glm::ivec3 image_size = downsample_image->get_attachment_config().extent;

						const uint32_t src_mip_width = glm::clamp(image_size.x >> src_index, 1, image_size.x);
						const uint32_t src_mip_height = glm::clamp(image_size.y >> src_index, 1, image_size.y);

						const uint32_t dst_mip_width = glm::clamp(image_size.x >> dst_index, 1, image_size.x);
						const uint32_t dst_mip_height = glm::clamp(image_size.y >> dst_index, 1, image_size.y);

						{
							BloomBlurData blur_data
							{
								.input_texture = (0x0000ffff & src_image_index),
								.output_texture = (0x0000ffff & dst_image_index),
								.input_texture_size = glm::vec2(src_mip_width, src_mip_height),
								.output_texture_size = glm::vec2(dst_mip_width, dst_mip_height)
							};
							PushConstantPipelineData pc_data = PushConstantPipelineData::create(&blur_data, PipelineShaderStageType::Compute);
							gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pc_data);
						}

						gfx_context->dispatch_compute(
							command_buffer,
							static_cast<uint32_t>((dst_mip_width + 31) / 32),
							static_cast<uint32_t>((dst_mip_height + 31) / 32),
							1
						);

						downsample_image->barrier(
							gfx_context,
							command_buffer,
							AccessFlags::ShaderWrite,
							AccessFlags::ShaderRead,
							i == 0 ? ImageLayout::Undefined : ImageLayout::General,
							ImageLayout::General,
							PipelineStageType::ComputeShader,
							PipelineStageType::ComputeShader
						);
					}
				}
			);

			render_graph.add_pass(
				gfx_context,
				"bloom_upsample_pass",
				RenderPassFlags::Compute,
				{
					.shader_setup = bloom_upsample_shader_setup,
					.inputs = { bloom_blur_image_desc },
					.outputs = { bloom_blur_image_desc },
					.b_split_input_image_mips = true
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					Image* const downsample_image = CACHE_FETCH(Image, graph.get_physical_resource(bloom_blur_image_desc));
					const uint32_t num_mips = downsample_image->get_num_image_views();

					for (int32_t i = num_mips - 1; i > 0; --i)
					{
						const uint32_t src_index = i;
						const uint32_t dst_index = i - 1;

						const BindingTableHandle src_image_index = frame_data.pass_bindless_resources.handles[src_index];
						const BindingTableHandle dst_image_index = frame_data.pass_bindless_resources.handles[num_mips + dst_index];

						glm::ivec3 image_size = downsample_image->get_attachment_config().extent;

						const uint32_t src_mip_width = glm::clamp(image_size.x >> src_index, 1, image_size.x);
						const uint32_t src_mip_height = glm::clamp(image_size.y >> src_index, 1, image_size.y);

						const uint32_t dst_mip_width = glm::clamp(image_size.x >> dst_index, 1, image_size.x);
						const uint32_t dst_mip_height = glm::clamp(image_size.y >> dst_index, 1, image_size.y);

						{
							BloomBlurData blur_data
							{
								.input_texture = (0x0000ffff & src_image_index),
								.output_texture = (0x0000ffff & dst_image_index),
								.input_texture_size = glm::vec2(src_mip_width, src_mip_height),
								.output_texture_size = glm::vec2(dst_mip_width, dst_mip_height),
								.bloom_filter_radius = 0.005f
							};
							PushConstantPipelineData pc_data = PushConstantPipelineData::create(&blur_data, PipelineShaderStageType::Compute);
							gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pc_data);
						}

						gfx_context->dispatch_compute(
							command_buffer,
							static_cast<uint32_t>((dst_mip_width + 31) / 32),
							static_cast<uint32_t>((dst_mip_height + 31) / 32),
							1
						);

						downsample_image->barrier(
							gfx_context,
							command_buffer,
							AccessFlags::ShaderWrite,
							AccessFlags::ShaderRead,
							ImageLayout::General,
							ImageLayout::General,
							PipelineStageType::ComputeShader,
							PipelineStageType::ComputeShader
						);
					}
				}
			);

			final_color_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "final_color",
					.format = Format::Float4x32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Image2D | ImageFlags::Sampled,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::Repeat,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true
				}
			);

			render_graph.add_pass(
				gfx_context,
				"bloom_resolve_pass",
				RenderPassFlags::Graphics,
				{
					.shader_setup = bloom_resolve_shader_setup,
					.inputs = { scene_color_desc, bloom_blur_image_desc },
					.outputs = { final_color_desc }
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					const BindingTableHandle color_image_handle = frame_data.pass_bindless_resources.handles[0];
					const BindingTableHandle bloom_brightness_color_handle = frame_data.pass_bindless_resources.handles[1];

					BloomResolveData resolve_data
					{
						.scene_color_texure = 0x0000ffff & color_image_handle,
						.bloom_brightness_texture = 0x0000ffff & bloom_brightness_color_handle,
						.exposure = static_cast<float>(cvar_final_image_exposure.get()),
						.bloom_intensity = static_cast<float>(cvar_bloom_intensity.get())
					};

					PushConstantPipelineData pass_data = PushConstantPipelineData::create(&resolve_data, PipelineShaderStageType::Fragment);

					gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pass_data);

					Renderer::get()->draw_fullscreen_quad(command_buffer);
				}
			);
		}

		// Full screen present pass
		{
			RGShaderDataSetup shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Vertex, "../../shaders/common/fullscreen.vert.sun" },
					{ PipelineShaderStageType::Fragment, "../../shaders/common/fullscreen.frag.sun"}
				},
				.attachment_blend = PipelineAttachmentBlendState
				{
					.b_blend_enabled = true,
					.source_color_blend = BlendFactor::SourceColor,
					.destination_color_blend = BlendFactor::Zero,
					.color_blend_op = BlendOp::Add,
				}
			};

			render_graph.add_pass(
				gfx_context,
				"present_pass",
				RenderPassFlags::Graphics | RenderPassFlags::Present,
				{
					.shader_setup = shader_setup,
					.inputs = { final_color_desc }
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					FullscreenData fullscreen_data
					{
						.scene_texture_index = (0x0000ffff & frame_data.pass_bindless_resources.handles[0])
					};

					PushConstantPipelineData pass_data = PushConstantPipelineData::create(&fullscreen_data, PipelineShaderStageType::Fragment);

					gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pass_data);

					Renderer::get()->draw_fullscreen_quad(command_buffer);
				}
			);
		}
#ifndef NDEBUG
		// IMGui pass
		//render_graph.add_pass(
		//	gfx_context,
		//	"tools_gui",
		//	RenderPassFlags::Graphics,
		//	[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
		//	{
		//		global_gui_core.initialize(gfx_context, gfx_context->get_window(), frame_data.current_pass);
		//		global_gui_core.new_frame(Renderer::get()->context()->get_window());
		//		global_gui_core.begin_draw();

		//		global_gui_core.end_draw(command_buffer);
		//	}
		//);
#endif 
		render_graph.submit(gfx_context, swapchain);
	}
}
