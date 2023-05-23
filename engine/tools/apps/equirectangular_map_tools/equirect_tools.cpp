#include <equirect_tools.h>
#include <window/window.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/image.h>
#include <graphics/renderer.h>
#include <graphics/command_queue.h>
#include <core/layers/scene.h>
#include <core/simulation_core.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <sstream>

#include <glm/gtc/matrix_transform.hpp>

namespace Sunset
{
	ImageID EquirectToolsApplication::equirect_image;
	ImageID EquirectToolsApplication::equirect_cubemap_image;
	ImageID EquirectToolsApplication::irradiance_map_image;
	ImageID EquirectToolsApplication::prefilter_map_image;
	ImageID EquirectToolsApplication::brdf_lut_image;

	class IBLBakingStrategy
	{
		struct EquirectCubemapConstants
		{
			glm::mat4 projection;
			glm::mat4 view;
			int32_t equirect_map_index;
			int32_t layer_index;
		};

		struct PrefilterCubemapConstants
		{
			glm::mat4 projection;
			glm::mat4 view;
			int32_t equirect_map_index;
			int32_t layer_index;
			float roughness;
			float source_cubemap_resolution;
		};

	public:
		void render(GraphicsContext* gfx_context, RenderGraph& render_graph, class Swapchain* swapchain, bool b_offline = false)
		{
			const glm::vec2 viewport_extent = Renderer::get()->context()->get_surface_resolution();

			RGResourceHandle equirect_image_desc = render_graph.register_image(
				gfx_context,
				EquirectToolsApplication::equirect_image
			);

			RGResourceHandle equirect_cubemap_image_desc = render_graph.register_image(
				gfx_context,
				EquirectToolsApplication::equirect_cubemap_image
			);

			RGResourceHandle environment_irradiance_image_desc = render_graph.register_image(
				gfx_context,
				EquirectToolsApplication::irradiance_map_image
			);

			RGResourceHandle environment_prefilter_image_desc = render_graph.register_image(
				gfx_context,
				EquirectToolsApplication::prefilter_map_image
			);

			RGResourceHandle brdf_lut_image_desc = render_graph.register_image(
				gfx_context,
				EquirectToolsApplication::brdf_lut_image
			);

			RGShaderDataSetup cubemap_shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Vertex, "../../shaders/tools/equirect_to_cubemap.vert.sun" },
					{ PipelineShaderStageType::Fragment, "../../shaders/tools/equirect_to_cubemap.frag.sun" }
				},
				.viewport = Viewport
				{
					.x = 0,
					.y = 0,
					.width = viewport_extent.x,
					.height = viewport_extent.x
				}
			};

			RGShaderDataSetup irradiance_shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Vertex, "../../shaders/tools/environment_irradiance.vert.sun" },
					{ PipelineShaderStageType::Fragment, "../../shaders/tools/environment_irradiance.frag.sun" }
				},
				.viewport = Viewport
				{
					.x = 0,
					.y = 0,
					.width = 32,
					.height = 32
				}
			};

			RGShaderDataSetup prefilter_shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Vertex, "../../shaders/tools/environment_prefilter.vert.sun" },
					{ PipelineShaderStageType::Fragment, "../../shaders/tools/environment_prefilter.frag.sun" }
				},
				.viewport = Viewport
				{
					.x = 0,
					.y = 0,
					.width = 128,
					.height = 128 
				}
			};

			RGShaderDataSetup brdf_shader_setup
			{
				.pipeline_shaders =
				{
					{ PipelineShaderStageType::Vertex, "../../shaders/common/fullscreen.vert.sun" },
					{ PipelineShaderStageType::Fragment, "../../shaders/tools/brdf_lut.frag.sun" }
				},
				.viewport = Viewport
				{
					.x = 0,
					.y = 0,
					.width = 512,
					.height = 512 
				}
			};

			const glm::mat4 capture_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

			const glm::mat4 capture_views[] =
			{
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
			};

			render_graph.add_pass(
				gfx_context,
				"equirect_to_cubemap_pass",
				RenderPassFlags::Graphics,
				{
					.shader_setup = cubemap_shader_setup,
					.inputs = { equirect_image_desc },
					.outputs = { equirect_cubemap_image_desc },
					.b_force_keep_pass = true
				},
				[gfx_context, capture_projection, capture_views](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					EquirectCubemapConstants equirect_to_cubemap_constants
					{
						.projection = capture_projection,
						.equirect_map_index = 0x0000ffff & frame_data.pass_bindless_resources.handles[0]
					};

					for (uint32_t i = 0; i < 6; ++i)
					{
						equirect_to_cubemap_constants.layer_index = static_cast<int32_t>(i);
						equirect_to_cubemap_constants.view = capture_views[i];

						PushConstantPipelineData pass_data = PushConstantPipelineData::create(&equirect_to_cubemap_constants, PipelineShaderStageType::Fragment | PipelineShaderStageType::Vertex);

						gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pass_data);

						Renderer::get()->draw_unit_cube(command_buffer);
					}
				}
			);

			render_graph.add_pass(
				gfx_context,
				"environment_irradiance_pass",
				RenderPassFlags::Graphics,
				{
					.shader_setup = irradiance_shader_setup,
					.inputs = { equirect_cubemap_image_desc },
					.outputs = { environment_irradiance_image_desc },
					.b_force_keep_pass = true
				},
				[gfx_context, capture_projection, capture_views](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					EquirectCubemapConstants equirect_to_cubemap_constants
					{
						.projection = capture_projection,
						.equirect_map_index = 0x0000ffff & frame_data.pass_bindless_resources.handles[0],
					};

					for (uint32_t i = 0; i < 6; ++i)
					{
						equirect_to_cubemap_constants.layer_index = static_cast<int32_t>(i);
						equirect_to_cubemap_constants.view = capture_views[i];

						PushConstantPipelineData pass_data = PushConstantPipelineData::create(&equirect_to_cubemap_constants, PipelineShaderStageType::Fragment | PipelineShaderStageType::Vertex);

						gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pass_data);

						Renderer::get()->draw_unit_cube(command_buffer);
					}
				}
			);

			{
				Image* const prefilter_image = CACHE_FETCH(Image, EquirectToolsApplication::prefilter_map_image);
				const glm::ivec3 prefilter_image_size = prefilter_image->get_attachment_config().extent;
				const uint32_t mip_count = prefilter_image->get_attachment_config().mip_count;

				for (uint32_t mip = 0; mip < mip_count; ++mip)
				{
					const uint32_t mip_width = glm::clamp(prefilter_image_size.x >> mip, 1, prefilter_image_size.x);
					const uint32_t mip_height = glm::clamp(prefilter_image_size.y >> mip, 1, prefilter_image_size.y);

					prefilter_shader_setup.viewport->width = mip_width;
					prefilter_shader_setup.viewport->height = mip_height;

					const std::string pass_name = "environment_prefilter_pass" + std::to_string(mip);

					const float mip_roughness = static_cast<float>(mip) / static_cast<float>(mip_count - 1);

					render_graph.add_pass(
						gfx_context,
						pass_name.c_str(),
						RenderPassFlags::Graphics,
						{
							.shader_setup = prefilter_shader_setup,
							.inputs = { equirect_cubemap_image_desc },
							.outputs = { environment_prefilter_image_desc },
							.output_views = { mip },
							.b_force_keep_pass = true
						},
						[gfx_context, capture_projection, capture_views, equirect_cubemap_image_desc, mip_roughness](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
						{
							Image* const equirect_cubemap_image = CACHE_FETCH(Image, graph.get_physical_resource(equirect_cubemap_image_desc));

							PrefilterCubemapConstants prefilter_cubemap_constants
							{
								.projection = capture_projection,
								.equirect_map_index = 0x0000ffff & frame_data.pass_bindless_resources.handles[0],
								.roughness = mip_roughness,
								.source_cubemap_resolution = equirect_cubemap_image->get_attachment_config().extent.x
							};

							for (uint32_t i = 0; i < 6; ++i)
							{
								prefilter_cubemap_constants.layer_index = static_cast<int32_t>(i);
								prefilter_cubemap_constants.view = capture_views[i];

								PushConstantPipelineData pass_data = PushConstantPipelineData::create(&prefilter_cubemap_constants, PipelineShaderStageType::Fragment | PipelineShaderStageType::Vertex);

								gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pass_data);

								Renderer::get()->draw_unit_cube(command_buffer);
							}
						}
					);
				}
			}

			render_graph.add_pass(
				gfx_context,
				"brdf_lut_pass",
				RenderPassFlags::Graphics,
				{
					.shader_setup = brdf_shader_setup,
					.outputs = { brdf_lut_image_desc },
					.b_force_keep_pass = true
				},
				[=](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
				{
					Renderer::get()->draw_fullscreen_quad(command_buffer);
				}
			);

			render_graph.submit(gfx_context, swapchain, b_offline);
		}
	};

	void EquirectToolsApplication::init(const std::filesystem::path& equirect_path, bool generate_cubemap, bool generate_irradiance_map, bool generate_prefilter_map, bool generate_brdf_lut)
	{
		Renderer::get()->setup(nullptr, glm::ivec2(1280, 1280));

		equirect_name = equirect_path.stem().string();
		parent_equirect_path = create_save_directory(equirect_path.parent_path());
		b_generate_cubemap_textures = generate_cubemap;
		b_generate_irradiance_map = generate_irradiance_map;
		b_generate_prefilter_map = generate_prefilter_map;
		b_generate_brdf_lut = generate_brdf_lut;

		load_equirect_image(equirect_path);

		create_equirect_cubemap();
		create_irradiance_map();
		create_prefilter_map();
		create_brdf_lut();

		SimulationCore::get()->register_layer(std::make_unique<Scene>());
	}

	void EquirectToolsApplication::cleanup()
	{	
		SimulationCore::get()->destroy();
		Renderer::get()->destroy();
	}

	void EquirectToolsApplication::run()
	{
		{
			Renderer::get()->wait_for_gpu();
			Renderer::get()->begin_frame();
			Renderer::get()->draw<IBLBakingStrategy>(true);
			Renderer::get()->wait_for_gpu();
		}

		// Save off output images
		if (b_generate_cubemap_textures)
		{
			write_ibl_texture_to_png(parent_equirect_path, "cubemap", EquirectToolsApplication::equirect_cubemap_image, true);
		}

		if (b_generate_irradiance_map)
		{
			write_ibl_texture_to_png(parent_equirect_path, "irradiance", EquirectToolsApplication::irradiance_map_image, false);
		}

		if (b_generate_prefilter_map)
		{
			write_ibl_texture_to_png(parent_equirect_path, "prefilter", EquirectToolsApplication::prefilter_map_image, false);
		}

		if (b_generate_brdf_lut)
		{
			write_ibl_texture_to_png(parent_equirect_path, "brdf_lut", EquirectToolsApplication::brdf_lut_image, true, true);
		}

		//bool bQuit = false;

		//while (!bQuit)
		//{
		//	Renderer::get()->context()->get_window()->poll();

		//	{
		//		ScopedRender<IBLBakingStrategy> scoped_render(Renderer::get());

		//		SimulationCore::get()->update();
		//	}
		//}
	}

	std::filesystem::path EquirectToolsApplication::create_save_directory(const std::filesystem::path& equirect_path)
	{
		const std::filesystem::path directory_path = equirect_path / equirect_name;
		std::filesystem::create_directory(directory_path);
		return directory_path;
	}

	void EquirectToolsApplication::load_equirect_image(const std::filesystem::path& equirect_path)
	{
		int x, y, n;
		if (float* data = stbi_loadf((const char*)equirect_path.u8string().c_str(), &x, &y, &n, 0))
		{
			const uint32_t pixel_stride = 4 * sizeof(float);

			const BufferID staging_buffer_id = BufferFactory::create(
				Renderer::get()->context(),
				{
					.name = "equirect_texture_staging_buffer",
					.buffer_size = x * y * pixel_stride,
					.type = BufferType::TransferSource,
					.memory_usage = MemoryUsageType::OnlyCPU
				},
				false
			);
			Buffer* const staging_buffer = CACHE_FETCH(Buffer, staging_buffer_id);

			staging_buffer->copy_from(
				Renderer::get()->context(),
				data,
				x * y * pixel_stride,
				0,
				[data, x, y, n](void* memory)
				{
					float* float_memory = (float*)memory;
					for (uint32_t i = 0; i < x; ++i)
					{
						for (uint32_t j = 0; j < y; ++j)
						{
							memcpy(float_memory + ((j * x + i) * 4), data + ((j * x + i) * n), 3 * sizeof(float));
						}
					}
				}
			);

			{
				AttachmentConfig config
				{
					.name = "equirect_texture",
					.format = Format::Float4x32,
					.extent = glm::vec3(x, y, 1.0f),
					.flags = ImageFlags::Color | ImageFlags::TransferDst | ImageFlags::Sampled,
					.usage_type = MemoryUsageType::OnlyGPU,
					.sampler_address_mode = SamplerAddressMode::EdgeClamp,
					.image_filter = ImageFilter::Linear
				};
				EquirectToolsApplication::equirect_image = ImageFactory::create(Renderer::get()->context(), config);

				Renderer::get()->context()->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(Renderer::get()->context(), [this, staging_buffer, gfx_context = Renderer::get()->context()](void* command_buffer)
				{
					Image* const image_obj = CACHE_FETCH(Image, EquirectToolsApplication::equirect_image);
					image_obj->copy_from_buffer(gfx_context, command_buffer, staging_buffer);
				});
			}

			CACHE_DELETE(Buffer, staging_buffer_id, Renderer::get()->context());
		}
	}

	void EquirectToolsApplication::create_equirect_cubemap()
	{
		const glm::vec2 viewport_extent = Renderer::get()->context()->get_surface_resolution();
		AttachmentConfig config
		{
			.name = "equirect_cubemap",
			.format = Format::Float4x32,
			.extent = glm::vec3(viewport_extent.x, viewport_extent.x, 1.0f),
			.flags = ImageFlags::Color | ImageFlags::Image2DArray | ImageFlags::Sampled | ImageFlags::TransferSrc,
			.usage_type = MemoryUsageType::OnlyGPU,
			.sampler_address_mode = SamplerAddressMode::EdgeClamp,
			.image_filter = ImageFilter::Linear,
			.mip_count = static_cast<uint32_t>(std::log2(Maths::npot(viewport_extent.x))),
			.array_count = 6
		};
		EquirectToolsApplication::equirect_cubemap_image = ImageFactory::create(Renderer::get()->context(), config);
	}

	void EquirectToolsApplication::create_irradiance_map()
	{
		AttachmentConfig config
		{
			.name = "irradiance_map",
			.format = Format::Float4x32,
			.extent = glm::vec3(32.0f, 32.0f, 1.0f),
			.flags = ImageFlags::Color | ImageFlags::Image2DArray | ImageFlags::Sampled | ImageFlags::TransferSrc,
			.usage_type = MemoryUsageType::OnlyGPU,
			.sampler_address_mode = SamplerAddressMode::EdgeClamp,
			.image_filter = ImageFilter::Linear,
			.array_count = 6
		};
		EquirectToolsApplication::irradiance_map_image = ImageFactory::create(Renderer::get()->context(), config);
	}

	void EquirectToolsApplication::create_prefilter_map()
	{
		AttachmentConfig config
		{
			.name = "prefilter_map",
			.format = Format::Float4x32,
			.extent = glm::vec3(128.0f, 128.0f, 1.0f),
			.flags = ImageFlags::Color | ImageFlags::Image2DArray | ImageFlags::Sampled | ImageFlags::TransferSrc,
			.usage_type = MemoryUsageType::OnlyGPU,
			.sampler_address_mode = SamplerAddressMode::EdgeClamp,
			.image_filter = ImageFilter::Linear,
			.mip_count = 5,
			.array_count = 6,
			.linear_mip_filtering = true
		};
		EquirectToolsApplication::prefilter_map_image = ImageFactory::create(Renderer::get()->context(), config);
	}

	void EquirectToolsApplication::create_brdf_lut()
	{
		AttachmentConfig config
		{
			.name = "brdf_lut",
			.format = Format::Float4x32,
			.extent = glm::vec3(512.0f, 512.0f, 1.0f),
			.flags = ImageFlags::Color | ImageFlags::Image2D | ImageFlags::Sampled | ImageFlags::TransferSrc,
			.usage_type = MemoryUsageType::OnlyGPU,
			.sampler_address_mode = SamplerAddressMode::EdgeClamp,
			.image_filter = ImageFilter::Linear
		};
		EquirectToolsApplication::brdf_lut_image = ImageFactory::create(Renderer::get()->context(), config);
	}

	void EquirectToolsApplication::write_ibl_texture_to_png(const std::filesystem::path& out_path, const char* texture_dir_name, ImageID texture_id, bool b_use_only_first_mip, bool b_flip_on_write)
	{
		const std::string out_directory = texture_dir_name;
		const std::filesystem::path out_texture_path = out_path / out_directory;
		std::filesystem::create_directory(out_texture_path);

		Image* const image_obj = CACHE_FETCH(Image, texture_id);

		const glm::ivec3 image_size = image_obj->get_attachment_config().extent;
		const uint32_t mip_count = b_use_only_first_mip ? 1 : image_obj->get_attachment_config().mip_count;
		const uint32_t layer_count = image_obj->get_attachment_config().array_count;
		const uint32_t image_channels = 4;

		const BufferID staging_buffer_id = BufferFactory::create(
			Renderer::get()->context(),
			{
				.name = "texture_transfer_buffer",
				.buffer_size = image_size.x * image_size.y * layer_count * mip_count * image_channels * sizeof(float),
				.type = BufferType::StorageBuffer | BufferType::TransferDestination,
				.memory_usage = MemoryUsageType::OnlyCPU
			},
			false
		);
		Buffer* const staging_buffer = CACHE_FETCH(Buffer, staging_buffer_id);

		// Copy generated cubemap texture to a staging buffer that we use to do our external image write
		Renderer::get()->context()->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(
			Renderer::get()->context(),
			[this, image_obj, image_size, layer_count, mip_count, image_channels, staging_buffer, gfx_context = Renderer::get()->context()](void* command_buffer)
			{
				uint32_t buffer_offset = 0;
				for (uint32_t j = 0; j < mip_count; ++j)
				{
					image_obj->copy_to_buffer(gfx_context, command_buffer, staging_buffer, buffer_offset, j, 0, layer_count);
					buffer_offset += (image_size.x >> j) * (image_size.y >> j) * image_channels * layer_count * sizeof(float);
				}
			});

		// Use the populated staging buffer to write each cubemap texture face to a .png
		{
			ScopedGPUBufferMapping buffer_mapping(Renderer::get()->context(), staging_buffer);

			uint32_t buffer_offset = 0;

			// Copy staging buffer to a temporary 8 bit buffer (since the .png format uses 8 bits per component)
			float* float_buffer = (float*)buffer_mapping.mapped_memory;
			uint8_t* pixel_buffer = new uint8_t[image_size.x * image_size.y * image_channels * mip_count * layer_count];
			for (uint32_t j = 0; j < mip_count; ++j)
			{
				for (uint32_t i = 0; i < layer_count; ++i)
				{
					const uint32_t width = (image_size.x >> j);
					const uint32_t height = (image_size.y >> j);

					for (uint32_t x = 0; x < width; ++x)
					{
						for (uint32_t y = 0; y < height; ++y)
						{
							const uint32_t pixel = buffer_offset + (y * width + x) * image_channels;
							pixel_buffer[pixel + 0] = static_cast<uint8_t>(glm::clamp(*(float_buffer + pixel + 0) * 255.0f, 0.0f, 255.0f)); // R
							pixel_buffer[pixel + 1] = static_cast<uint8_t>(glm::clamp(*(float_buffer + pixel + 1) * 255.0f, 0.0f, 255.0f)); // G
							pixel_buffer[pixel + 2] = static_cast<uint8_t>(glm::clamp(*(float_buffer + pixel + 2) * 255.0f, 0.0f, 255.0f)); // B
							pixel_buffer[pixel + 3] = static_cast<uint8_t>(glm::clamp(*(float_buffer + pixel + 3) * 255.0f, 0.0f, 255.0f)); // A
						}
					}

					buffer_offset += (width * height * image_channels);
				}
			}

			if (b_flip_on_write)
			{
				stbi_flip_vertically_on_write(true);
			}

			// Write our temporary 8 bit buffer to a set of .png images
			buffer_offset = 0;
			for (uint32_t j = 0; j < mip_count; ++j)
			{
				for (uint32_t i = 0; i < layer_count; ++i)
				{
					const uint32_t width = (image_size.x >> j);
					const uint32_t height = (image_size.y >> j);
					const uint32_t buffer_size = width * height * image_channels;
					const uint32_t byte_stride = width * image_channels;

					std::ostringstream filename_stream;
					filename_stream << equirect_name << "_" << texture_dir_name << "_mip_" << std::to_string(j) << "_layer_" << std::to_string(i) << ".png";

					std::filesystem::path new_path = out_texture_path / filename_stream.str();

					stbi_write_png(
						new_path.string().c_str(),
						width,
						height,
						image_channels,
						pixel_buffer + buffer_offset,
						byte_stride
					);

					buffer_offset += buffer_size;
				}
			}

			delete[] pixel_buffer;
		}

		CACHE_DELETE(Buffer, staging_buffer_id, Renderer::get()->context());
	}
}

