#include <graphics/strategies/deferred_shading.h>
#include <graphics/graphics_context.h>
#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <graphics/descriptor.h>
#include <graphics/resource/image.h>
#include <graphics/command_queue.h>
#include <window/window.h>
#include <core/data_globals.h>
#include <utility/maths.h>
#include <utility/cvar.h>
#ifndef NDEBUG
#include <utility/gui/gui_core.h>
#endif

#include <random>

namespace Sunset
{
	AutoCVar_Bool cvar_use_skybox("ren.use_skybox", "Whether or not to render a skybox (supplied to the scene as a cubemap texture) instead of using the preetham skydome shader.", false);

	AutoCVar_Bool cvar_ssao_enabled("ren.ssao.enable", "Whether or not to do screen space ambient occlusion", true);
	AutoCVar_Float cvar_ssao_strength("ren.ssao.strength", "The strength of the SSAO contribution applied to ambient lighting calculations", 2.0f);
	AutoCVar_Float cvar_ssao_radius("ren.ssao.radius", "The contribution radius of SSAO samples", 1.0f);

	AutoCVar_Bool cvar_ssr_enabled("ren.ssr.enable", "Whether or not to do screen space reflections", true);

	AutoCVar_Int cvar_num_bloom_pass_iterations("ren.bloom.num_pass_iterations", "The number of bloom horizontal and vertical blur iterations. (0 to turn bloom off).", 6);
	AutoCVar_Float cvar_bloom_intensity("ren.bloom.intensity", "The intensity of the applied final bloom", 0.2f);

	AutoCVar_Int cvar_taa_inverse_luminance_filter("ren.taa.inverse_luminance_filter_enabled", "Whether to do inverse luminance filtering during the history color resolve", 1);
	AutoCVar_Int cvar_taa_luminance_diff_filter("ren.taa.luminance_diff_enabled", "Whether to do luminance difference filtering during the history color resolve", 1);

	AutoCVar_Float cvar_final_image_exposure("ren.final_image_exposure", "The exposure to apply once HDR color gets resolved down to LDR", 2.0f);

	void DeferredShadingStrategy::render(GraphicsContext* gfx_context, RenderGraph& render_graph, class Swapchain* swapchain, bool b_offline)
	{
		DeferredShadingPersistentStorage::get()->initialize();

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

		RGResourceHandle ssao_data_buffer_desc = render_graph.register_buffer(
			gfx_context,
			DeferredShadingPersistentStorage::get()->ssao_data_buffer[buffered_frame_number]
		);

		RGResourceHandle hzb_image_desc = render_graph.register_image(
			gfx_context,
			Renderer::get()->get_persistent_image("hi_z", buffered_frame_number)
		);

		RGResourceHandle temporal_color_history = render_graph.register_image(
			gfx_context,
			Renderer::get()->get_persistent_image("temporal_color_history", buffered_frame_number)
		);

		RGResourceHandle ssao_noise_image_desc = render_graph.register_image(
			gfx_context,
			Renderer::get()->get_persistent_image("ssao_noise", buffered_frame_number)
		);

		// Mesh cull pass
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
						draw_cull_data.hzb_texture = 0x0000ffff & frame_data.pass_bindless_resources.handles.front();
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

		// Skydome pass
		RGResourceHandle sky_image_desc;
		{
			const glm::vec2 image_extent = gfx_context->get_surface_resolution();
			sky_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "sky_color",
					.format = Format::Float4x32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Image2D | ImageFlags::Sampled,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::Repeat,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true
				}
			);

			if (cvar_use_skybox.get())
			{
				RGShaderDataSetup shader_setup
				{
					.pipeline_shaders =
					{
						{ PipelineShaderStageType::Vertex, "../../shaders/common/skybox.vert.sun" },
						{ PipelineShaderStageType::Fragment, "../../shaders/common/skybox.frag.sun"}
					},
					.b_depth_write_enabled = false
				};

				render_graph.add_pass(
					gfx_context,
					"skybox_pass",
					RenderPassFlags::Graphics,
					{
						.shader_setup = shader_setup,
						.outputs = { sky_image_desc }
					},
					[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
					{
						Renderer::get()->draw_unit_cube(command_buffer);
					}
				);
			}
			else
			{
				RGShaderDataSetup shader_setup
				{
					.pipeline_shaders =
					{
						{ PipelineShaderStageType::Vertex, "../../shaders/common/skydome.vert.sun" },
						{ PipelineShaderStageType::Fragment, "../../shaders/common/skydome.frag.sun"}
					},
					.b_depth_write_enabled = false
				};

				render_graph.add_pass(
					gfx_context,
					"skydome_pass",
					RenderPassFlags::Graphics,
					{
						.shader_setup = shader_setup,
						.outputs = { sky_image_desc }
					},
					[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
					{
						Renderer::get()->draw_unit_sphere(command_buffer);
					}
				);
			}
		}

		// G-Buffer pass
		RGResourceHandle main_albedo_image_desc;
		RGResourceHandle main_depth_image_desc;
		RGResourceHandle main_smra_image_desc;
		RGResourceHandle main_cc_image_desc;
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
			const glm::vec2 image_extent = gfx_context->get_surface_resolution();
			main_albedo_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "main_albedo",
					.format = Format::Float4x32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
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
					.attachment_stencil_clear = true
				}
			);
			main_smra_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "main_smra",
					.format = Format::Float4x32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Sampled | ImageFlags::Image2D,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::EdgeClamp,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true,
				}
			);
			main_cc_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "main_cc",
					.format = Format::Float2x32,
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
					.image_filter = ImageFilter::Nearest,
					.attachment_clear = true,
				}
			);

			RGPassHandle base_pass_handle = render_graph.add_pass(
				gfx_context,
				"gbuffer_base_pass",
				RenderPassFlags::Graphics,
				{
					.shader_setup = shader_setup,
					.inputs = { entity_data_buffer_desc, material_data_buffer_desc,
								compacted_object_instance_buffer_desc,
								draw_indirect_buffer_desc },
					.outputs = { main_albedo_image_desc, main_depth_image_desc, main_smra_image_desc, main_cc_image_desc,
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

		// Motion vectors pass
		RGResourceHandle motion_vectors_image_desc;
		{
			RGShaderDataSetup shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Compute, "../../shaders/common/camera_motion_vectors.comp.sun" }
				}
			};

			const glm::vec2 image_extent = gfx_context->get_surface_resolution();
			motion_vectors_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "motion_vectors",
					.format = Format::Float2x16,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Sampled | ImageFlags::Storage | ImageFlags::Image2D,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::EdgeClamp,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true
				}
			);

			render_graph.add_pass(
				gfx_context,
				"compute_motion_vectors",
				RenderPassFlags::Compute,
				{
					.shader_setup = shader_setup,
					.inputs = { main_depth_image_desc, motion_vectors_image_desc },
					.outputs = { motion_vectors_image_desc },
					.b_split_input_image_mips = true
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					const BindingTableHandle depth_image_index = frame_data.pass_bindless_resources.handles[0];
					const BindingTableHandle motion_vectors_image_index = frame_data.pass_bindless_resources.handles[2];

					{
						MotionVectorsData motion_vectors_data
						{
							.input_depth_index = (0x0000ffff & depth_image_index),
							.output_motion_vectors_index = (0x0000ffff & motion_vectors_image_index),
							.resolution = image_extent
						};
						PushConstantPipelineData pc_data = PushConstantPipelineData::create(&motion_vectors_data, PipelineShaderStageType::Compute);
						gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pc_data);
					}

					gfx_context->dispatch_compute(
						command_buffer,
						static_cast<uint32_t>((image_extent.x + 15) / 16),
						static_cast<uint32_t>((image_extent.y + 15) / 16),
						1
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

		// SSAO pass
		RGResourceHandle ssao_image_desc{ 0 };
		if (cvar_ssao_enabled.get())
		{
			RGShaderDataSetup ssao_shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Compute, "../../shaders/effects/ssao.comp.sun" }
				}
			};

			RGShaderDataSetup ssao_blur_shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Compute, "../../shaders/effects/box_blur.comp.sun" }
				}
			};

			const glm::vec2 image_extent = gfx_context->get_surface_resolution();
			RGResourceHandle ssao_staging_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "ssao_staging_image",
					.format = Format::Float32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Image2D | ImageFlags::Sampled | ImageFlags::Storage,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::Repeat,
					.image_filter = ImageFilter::Nearest,
					.attachment_clear = true
				}
			);

			ssao_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "ssao_image",
					.format = Format::Float32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Image2D | ImageFlags::Sampled | ImageFlags::Storage,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::Repeat,
					.image_filter = ImageFilter::Nearest,
					.attachment_clear = true
				}
			);

			render_graph.add_pass(
				gfx_context,
				"ssao_pass",
				RenderPassFlags::Compute,
				{
					.shader_setup = ssao_shader_setup,
					.inputs = { ssao_data_buffer_desc, ssao_noise_image_desc, main_position_image_desc, main_normal_image_desc, ssao_staging_image_desc },
					.outputs = { ssao_staging_image_desc }
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					const BindingTableHandle ssao_noise_image_handle = frame_data.pass_bindless_resources.handles[0];
					const BindingTableHandle position_image_handle = frame_data.pass_bindless_resources.handles[1];
					const BindingTableHandle normal_image_handle = frame_data.pass_bindless_resources.handles[2];
					const BindingTableHandle ssao_image_handle = frame_data.pass_bindless_resources.handles[4];

					SSAOData ssao_data 
					{
						.ssao_noise_texture = (0x0000ffff & ssao_noise_image_handle),
						.position_texture = (0x0000ffff & position_image_handle),
						.normal_texture = (0x0000ffff & normal_image_handle),
						.out_ssao_texture = (0x0000ffff & ssao_image_handle),
						.noise_scale = glm::vec2(image_extent.x * 0.25f, image_extent.y * 0.25f),
						.resolution = image_extent,
						.radius = static_cast<float>(cvar_ssao_radius.get()),
						.bias = 0.05f,
						.strength = static_cast<float>(cvar_ssao_strength.get())
					};

					PushConstantPipelineData pass_data = PushConstantPipelineData::create(&ssao_data, PipelineShaderStageType::Compute);

					gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pass_data);
					
					gfx_context->dispatch_compute(
						command_buffer,
						static_cast<uint32_t>((image_extent.x + 15) / 16),
						static_cast<uint32_t>((image_extent.y + 15) / 16),
						1
					);
				}
			);

			render_graph.add_pass(
				gfx_context,
				"ssao_blur_pass",
				RenderPassFlags::Compute,
				{
					.shader_setup = ssao_blur_shader_setup,
					.inputs = { ssao_staging_image_desc, ssao_image_desc },
					.outputs = { ssao_image_desc }
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					const BindingTableHandle ssao_staging_image_handle = frame_data.pass_bindless_resources.handles[0];
					const BindingTableHandle ssao_image_handle = frame_data.pass_bindless_resources.handles[3];

					BoxBlurData ssao_data
					{
						.staging_texture = (0x0000ffff & ssao_staging_image_handle),
						.out_texture = (0x0000ffff & ssao_image_handle),
						.resolution = image_extent
					};

					PushConstantPipelineData pass_data = PushConstantPipelineData::create(&ssao_data, PipelineShaderStageType::Compute);

					gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pass_data);

					gfx_context->dispatch_compute(
						command_buffer,
						static_cast<uint32_t>((image_extent.x + 15) / 16),
						static_cast<uint32_t>((image_extent.y + 15) / 16),
						1
					);
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
					{ PipelineShaderStageType::Fragment, "../../shaders/deferred/deferred_lit_standard.frag.sun"}
				}
			};

			const glm::vec2 image_extent = gfx_context->get_surface_resolution();
			scene_color_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "scene_color",
					.format = Format::Float4x32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Image2D | ImageFlags::Sampled | ImageFlags::Storage,
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
								main_smra_image_desc, main_cc_image_desc, main_normal_image_desc,
								main_position_image_desc, sky_image_desc, ssao_image_desc },
					.outputs = { scene_color_desc }
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					const BindingTableHandle albedo_image_handle = frame_data.pass_bindless_resources.handles[0];
					const BindingTableHandle smra_image_handle = frame_data.pass_bindless_resources.handles[1];
					const BindingTableHandle cc_image_handle = frame_data.pass_bindless_resources.handles[2];
					const BindingTableHandle normal_image_handle = frame_data.pass_bindless_resources.handles[3];
					const BindingTableHandle position_image_handle = frame_data.pass_bindless_resources.handles[4];
					const BindingTableHandle sky_image_handle = frame_data.pass_bindless_resources.handles[5];

					uint8_t next_bindless_index = 6;
					const BindingTableHandle ssao_image_handle = ssao_image_desc != 0 ? frame_data.pass_bindless_resources.handles[next_bindless_index++] : -1;

					LightingPassData lighting_data
					{
						.albedo_texure = 0x0000ffff & albedo_image_handle,
						.smra_texure = 0x0000ffff & smra_image_handle,
						.cc_texture = 0x0000ffff & cc_image_handle,
						.normal_texure = 0x0000ffff & normal_image_handle,
						.position_texure = 0x0000ffff & position_image_handle,
						.sky_texure = 0x0000ffff & sky_image_handle,
						.ssao_texture = ssao_image_handle >= 0 ? 0x0000ffff & ssao_image_handle : -1
					};

					PushConstantPipelineData pass_data = PushConstantPipelineData::create(&lighting_data, PipelineShaderStageType::Fragment);

					gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pass_data);

					Renderer::get()->draw_fullscreen_quad(command_buffer);
				}
			);
		}

		// TAA pass
		{
			RGShaderDataSetup shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Compute, "../../shaders/effects/taa.comp.sun" }
				}
			};

			const glm::vec2 image_extent = gfx_context->get_surface_resolution();
			const uint32_t current_frame_number = gfx_context->get_frame_number();

			render_graph.add_pass(
				gfx_context,
				"temporal_aa",
				RenderPassFlags::Compute,
				{
					.shader_setup = shader_setup,
					.inputs = { scene_color_desc, temporal_color_history, motion_vectors_image_desc, main_depth_image_desc },
					.outputs = { scene_color_desc, temporal_color_history }
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					const BindingTableHandle scene_color_index = frame_data.pass_bindless_resources.handles[0];
					const BindingTableHandle history_color_index = frame_data.pass_bindless_resources.handles[1];
					const BindingTableHandle motion_vectors_index = frame_data.pass_bindless_resources.handles[2];
					const BindingTableHandle depth_index = frame_data.pass_bindless_resources.handles[3];
					const BindingTableHandle out_scene_color_index = frame_data.pass_bindless_resources.handles[4];
					const BindingTableHandle out_history_color_index = frame_data.pass_bindless_resources.handles[5];

					{
						TAAData temporal_aa_data
						{
							.input_scene_color = (0x0000ffff & scene_color_index),
							.input_color_history = (0x0000ffff & history_color_index),
							.input_motion_vectors = (0x0000ffff & motion_vectors_index),
							.input_depth = (0x0000ffff & depth_index),
							.output_scene_color = (0x0000ffff & out_scene_color_index),
							.output_color_history = (0x0000ffff & out_history_color_index),
							.inverse_luminance_filter_enabled = cvar_taa_inverse_luminance_filter.get(),
							.luminance_difference_filter_enabled = cvar_taa_luminance_diff_filter.get(),
							.resolution = image_extent
						};
						PushConstantPipelineData pc_data = PushConstantPipelineData::create(&temporal_aa_data, PipelineShaderStageType::Compute);
						gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pc_data);
					}

					gfx_context->dispatch_compute(
						command_buffer,
						static_cast<uint32_t>((image_extent.x + 15) / 16),
						static_cast<uint32_t>((image_extent.y + 15) / 16),
						1
					);
				}
			);
		}

		RGResourceHandle post_bloom_color_desc = scene_color_desc;

		// Bloom pass
		const uint32_t num_iterations = cvar_num_bloom_pass_iterations.get();
		if (num_iterations > 0)
		{
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

			const glm::vec2 image_extent = gfx_context->get_surface_resolution();
			const float extent_x = Maths::npot(image_extent.x);
			const float extent_y = Maths::npot(image_extent.y);

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
						const BindingTableHandle dst_image_index = frame_data.pass_bindless_resources.handles[2 + num_mips + dst_index];

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

			post_bloom_color_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "post_bloom_color",
					.format = Format::Float4x32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Image2D | ImageFlags::Sampled | ImageFlags::Storage,
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
					.outputs = { post_bloom_color_desc }
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

		RGResourceHandle post_ssr_color_desc = post_bloom_color_desc;

		// SSR Pass
		if (cvar_ssr_enabled.get())
		{
			RGShaderDataSetup ssr_shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Compute, "../../shaders/effects/ssr.comp.sun" }
				}
			};

			RGShaderDataSetup ssr_blur_shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Compute, "../../shaders/effects/box_blur.comp.sun" }
				}
			};

			RGShaderDataSetup ssr_resolve_shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Vertex, "../../shaders/common/fullscreen.vert.sun" },
					{ PipelineShaderStageType::Fragment, "../../shaders/effects/ssr_resolve.frag.sun" }
				}
			};

			const glm::vec2 image_extent = gfx_context->get_surface_resolution();
			RGResourceHandle ssr_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "ssr_image",
					.format = Format::Float4x32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Image2D | ImageFlags::Sampled | ImageFlags::Storage,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::Repeat,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true
				}
			);

			RGResourceHandle ssr_blurred_image_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "ssr_blurred_image",
					.format = Format::Float4x32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Image2D | ImageFlags::Sampled | ImageFlags::Storage,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::Repeat,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true
				}
			);

			render_graph.add_pass(
				gfx_context,
				"ssr_pass",
				RenderPassFlags::Compute,
				{
					.shader_setup = ssr_shader_setup,
					.inputs = { main_position_image_desc, main_normal_image_desc, main_smra_image_desc,
								scene_color_desc, ssr_image_desc },
					.outputs = { ssr_image_desc }
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					const BindingTableHandle position_image_handle = frame_data.pass_bindless_resources.handles[0];
					const BindingTableHandle normal_image_handle = frame_data.pass_bindless_resources.handles[1];
					const BindingTableHandle smra_image_handle = frame_data.pass_bindless_resources.handles[2];
					const BindingTableHandle scene_color_image_handle = frame_data.pass_bindless_resources.handles[3];
					const BindingTableHandle ssr_image_handle = frame_data.pass_bindless_resources.handles[6];

					SSRData ssr_data
					{
						.scene_color_texture = (0x0000ffff & scene_color_image_handle),
						.position_texture = (0x0000ffff & position_image_handle),
						.normal_texture = (0x0000ffff & normal_image_handle),
						.smra_texture = (0x0000ffff & smra_image_handle),
						.out_ssr_texture = (0x0000ffff & ssr_image_handle),
						.ray_hit_steps = 24,
						.max_ray_distance = 16.0f,
						.resolution = image_extent,
					};

					PushConstantPipelineData pass_data = PushConstantPipelineData::create(&ssr_data, PipelineShaderStageType::Compute);

					gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pass_data);

					gfx_context->dispatch_compute(
						command_buffer,
						static_cast<uint32_t>((image_extent.x + 15) / 16),
						static_cast<uint32_t>((image_extent.y + 15) / 16),
						1
					);
				}
			);

			render_graph.add_pass(
				gfx_context,
				"ssr_blur_pass",
				RenderPassFlags::Compute,
				{
					.shader_setup = ssr_blur_shader_setup,
					.inputs = { ssr_image_desc, ssr_blurred_image_desc },
					.outputs = { ssr_blurred_image_desc }
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					const BindingTableHandle ssr_image_handle = frame_data.pass_bindless_resources.handles[0];
					const BindingTableHandle ssr_blurred_image_handle = frame_data.pass_bindless_resources.handles[3];

					BoxBlurData blur_data
					{
						.staging_texture = (0x0000ffff & ssr_image_handle),
						.out_texture = (0x0000ffff & ssr_blurred_image_handle),
						.resolution = image_extent
					};

					PushConstantPipelineData pass_data = PushConstantPipelineData::create(&blur_data, PipelineShaderStageType::Compute);

					gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pass_data);

					gfx_context->dispatch_compute(
						command_buffer,
						static_cast<uint32_t>((image_extent.x + 15) / 16),
						static_cast<uint32_t>((image_extent.y + 15) / 16),
						1
					);
				}
			);

			post_ssr_color_desc = render_graph.create_image(
				gfx_context,
				{
					.name = "post_ssr_color",
					.format = Format::Float4x32,
					.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::Image2D | ImageFlags::Sampled | ImageFlags::Storage,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::EdgeClamp,
					.image_filter = ImageFilter::Linear,
					.attachment_clear = true
				}
			);

			render_graph.add_pass(
				gfx_context,
				"ssr_resolve_pass",
				RenderPassFlags::Graphics,
				{
					.shader_setup = ssr_resolve_shader_setup,
					.inputs = { main_smra_image_desc, ssr_image_desc, ssr_blurred_image_desc, post_bloom_color_desc },
					.outputs = { post_ssr_color_desc }
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					SSRResolveData fullscreen_data
					{
						.smra_texture = (0x0000ffff & frame_data.pass_bindless_resources.handles[0]),
						.ssr_texture = (0x0000ffff & frame_data.pass_bindless_resources.handles[1]),
						.ssr_blurred_texture = (0x0000ffff & frame_data.pass_bindless_resources.handles[2]),
						.scene_color_texture = (0x0000ffff & frame_data.pass_bindless_resources.handles[3]),
						.ssr_strength = 2.0f
					};

					PushConstantPipelineData pass_data = PushConstantPipelineData::create(&fullscreen_data, PipelineShaderStageType::Fragment);

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
					.inputs = { post_ssr_color_desc }
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
		render_graph.submit(gfx_context, swapchain, b_offline);
	}

	void DeferredShadingPersistentStorage::initialize()
	{
		if (!b_initialized)
		{
			GraphicsContext* const gfx_context = Renderer::get()->context();
			const glm::vec2 image_extent = gfx_context->get_surface_resolution();

			{
				uint32_t image_width_npot = Maths::npot(image_extent.x);
				uint32_t image_height_npot = Maths::npot(image_extent.y);
				uint32_t mip_levels = glm::max(std::log2(image_width_npot), std::log2(image_height_npot));

				Renderer::get()->register_persistent_image(
					"hi_z",
					ImageFactory::create(
						gfx_context,
						{
							.name = "hi_z",
							.format = Format::Float32,
							.extent = glm::vec3(image_width_npot, image_height_npot, 1.0f),
							.flags = ImageFlags::Color | ImageFlags::Sampled | ImageFlags::Storage | ImageFlags::TransferDst | ImageFlags::TransferSrc,
							.usage_type = MemoryUsageType::OnlyGPU,
							.sampler_address_mode = SamplerAddressMode::EdgeClamp,
							.image_filter = ImageFilter::Linear,
							.mip_count = mip_levels,
							.attachment_clear = true,
							.attachment_stencil_clear = true,
							.does_min_reduction = true
						}
					)
				);
			}

			{
				Renderer::get()->register_persistent_image(
					"temporal_color_history",
					ImageFactory::create(
						gfx_context,
						{
							.name = "temporal_color_history",
							.format = Format::Float4x32,
							.extent = glm::vec3(image_extent.x, image_extent.y, 1.0f),
							.flags = ImageFlags::Color | ImageFlags::Sampled | ImageFlags::Storage,
							.usage_type = MemoryUsageType::OnlyGPU,
							.sampler_address_mode = SamplerAddressMode::EdgeClamp,
							.image_filter = ImageFilter::Linear,
							.attachment_clear = true
						}
						)
				);
			}

			{
				Renderer::get()->register_persistent_image(
					"ssao_noise",
					ImageFactory::create(
						gfx_context,
						{
							.name = "ssao_noise",
							.format = Format::Float4x32,
							.extent = glm::vec3(4.0f, 4.0f, 1.0f),
							.flags = ImageFlags::Color | ImageFlags::Sampled | ImageFlags::TransferDst,
							.usage_type = MemoryUsageType::OnlyGPU,
							.sampler_address_mode = SamplerAddressMode::Repeat,
							.image_filter = ImageFilter::Nearest,
							.attachment_clear = true
						}
					)
				);

				BufferID staging_buffer_id = BufferFactory::create(
					gfx_context,
					{
						.name = "ssao_noise_staging",
						.buffer_size = 16 * sizeof(glm::vec4),
						.type = BufferType::TransferSource,
						.memory_usage = MemoryUsageType::CPUToGPU 
					},
					false
				);

				std::uniform_real_distribution<float> random_floats(0.0f, 1.0f);
				std::default_random_engine rng;

				glm::vec4 ssao_noise_data[16];
				for (uint32_t i = 0; i < 16; ++i)
				{
					ssao_noise_data[i] = glm::vec4(random_floats(rng) * 2.0f - 1.0f, random_floats(rng) * 2.0f - 1.0f, 0.0f, 0.0f);
				}

				Buffer* const ssao_noise_staging_buffer = CACHE_FETCH(Buffer, staging_buffer_id);
				ssao_noise_staging_buffer->copy_from(
					gfx_context,
					&ssao_noise_data,
					16 * sizeof(glm::vec4)
				);

				gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(Renderer::get()->context(), [ssao_noise_staging_buffer, gfx_context](void* command_buffer)
				{
					for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
					{
						const ImageID ssao_noise_image = Renderer::get()->get_persistent_image("ssao_noise", i);
						CACHE_FETCH(Image, ssao_noise_image)->copy_from_buffer(gfx_context, command_buffer, ssao_noise_staging_buffer);
					}
				});

				CACHE_DELETE(Buffer, staging_buffer_id, gfx_context);
			}

			{
				std::uniform_real_distribution<float> random_floats(0.0f, 1.0f);
				std::default_random_engine rng;
				SSAOKernelData ssao_data;

				for (uint32_t i = 0; i < SSAOKernelSize; ++i)
				{
					ssao_data.kernel[i] = glm::vec3(random_floats(rng) * 2.0f - 1.0f, random_floats(rng) * 2.0f - 1.0f, random_floats(rng));
					ssao_data.kernel[i] = glm::normalize(ssao_data.kernel[i]);
					ssao_data.kernel[i] *= random_floats(rng);

					float scale = static_cast<float>(i) / static_cast<float>(SSAOKernelSize);
					scale = 0.1f + (scale * scale) * (1.0f - 0.1f);
					ssao_data.kernel[i] *= scale;
				}

				for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i)
				{
					ssao_data_buffer[i] = BufferFactory::create(
						gfx_context,
						{
							.name = "ssao_data",
							.buffer_size = sizeof(SSAOKernelData),
							.type = BufferType::StorageBuffer
						}
					);

					Buffer* const ssao_buffer = CACHE_FETCH(Buffer, ssao_data_buffer[i]);
					ssao_buffer->copy_from(
						gfx_context,
						&ssao_data,
						sizeof(SSAOKernelData)
					);
				}
			}

			b_initialized = true;
		}
	}
}
