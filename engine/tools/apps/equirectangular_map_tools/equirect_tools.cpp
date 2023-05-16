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

	class IBLBakingStrategy
	{
		struct EquirectCubemapConstants
		{
			glm::mat4 projection;
			glm::mat4 view;
			int32_t equirect_map_index;
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

			const glm::mat4 capture_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

			const glm::mat4 capture_views[] =
			{
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
			};

			for (uint32_t i = 0; i < 6; ++i)
			{
				const std::string pass_name = "equirect_to_cubemap_pass" + std::to_string(i);
				render_graph.add_pass(
					gfx_context,
					pass_name.c_str(),
					RenderPassFlags::Graphics,
					{
						.shader_setup = cubemap_shader_setup,
						.inputs = { equirect_image_desc },
						.outputs = { equirect_cubemap_image_desc },
						.output_layers = { i },
						.b_force_keep_pass = true
					},
					[gfx_context, capture_projection, view = capture_views[i]](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
					{
						EquirectCubemapConstants equirect_to_cubemap_constants
						{
							.projection = capture_projection,
							.view = view,
							.equirect_map_index = 0x0000ffff & frame_data.pass_bindless_resources.handles[0]
						};

						PushConstantPipelineData pass_data = PushConstantPipelineData::create(&equirect_to_cubemap_constants, PipelineShaderStageType::Fragment | PipelineShaderStageType::Vertex);

						gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pass_data);
						
						Renderer::get()->draw_unit_cube(command_buffer);
					}
				);
			}

			for (uint32_t i = 0; i < 6; ++i)
			{
				const std::string pass_name = "environment_irradiance_pass" + std::to_string(i);
				render_graph.add_pass(
					gfx_context,
					pass_name.c_str(),
					RenderPassFlags::Graphics,
					{
						.shader_setup = irradiance_shader_setup,
						.inputs = { equirect_cubemap_image_desc },
						.outputs = { environment_irradiance_image_desc },
						.output_layers = { i },
						.b_force_keep_pass = true
					},
					[gfx_context, capture_projection, view = capture_views[i]](RenderGraph& graph, RGFrameData& frame_data, void* command_buffer)
					{
						EquirectCubemapConstants equirect_to_cubemap_constants
						{
							.projection = capture_projection,
							.view = view,
							.equirect_map_index = 0x0000ffff & frame_data.pass_bindless_resources.handles[0]
						};

						PushConstantPipelineData pass_data = PushConstantPipelineData::create(&equirect_to_cubemap_constants, PipelineShaderStageType::Fragment | PipelineShaderStageType::Vertex);

						gfx_context->push_constants(command_buffer, frame_data.pass_pipeline_state, pass_data);

						Renderer::get()->draw_unit_cube(command_buffer);
					}
				);
			}

			render_graph.submit(gfx_context, swapchain, b_offline);
		}
	};

	void EquirectToolsApplication::init(const std::filesystem::path& equirect_path, bool generate_cubemap, bool generate_irradiance_map)
	{
		Renderer::get()->setup(nullptr, glm::ivec2(1280, 1280));

		equirect_name = equirect_path.stem().string();
		parent_equirect_path = create_save_directory(equirect_path.parent_path());
		b_generate_cubemap_textures = generate_cubemap;
		b_generate_irradiance_map = generate_irradiance_map;

		load_equirect_image(equirect_path);
		create_equirect_cubemap();
		create_irradiance_map();

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
			write_cubemap_to_png(parent_equirect_path);
		}

		if (b_generate_irradiance_map)
		{
			write_irradiance_to_png(parent_equirect_path);
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
			.flags = ImageFlags::Color | ImageFlags::Cube | ImageFlags::Sampled | ImageFlags::TransferSrc,
			.usage_type = MemoryUsageType::OnlyGPU,
			.sampler_address_mode = SamplerAddressMode::EdgeClamp,
			.image_filter = ImageFilter::Linear,
			.array_count = 6
		};
		EquirectToolsApplication::equirect_cubemap_image = ImageFactory::create(Renderer::get()->context(), config);
	}

	void EquirectToolsApplication::create_irradiance_map()
	{
		const glm::vec2 viewport_extent = Renderer::get()->context()->get_surface_resolution();
		AttachmentConfig config
		{
			.name = "irradiance_map",
			.format = Format::Float4x32,
			.extent = glm::vec3(32.0f, 32.0f, 1.0f),
			.flags = ImageFlags::Color | ImageFlags::Image2D | ImageFlags::Sampled | ImageFlags::TransferSrc,
			.usage_type = MemoryUsageType::OnlyGPU,
			.sampler_address_mode = SamplerAddressMode::EdgeClamp,
			.image_filter = ImageFilter::Linear,
			.array_count = 6
		};
		EquirectToolsApplication::irradiance_map_image = ImageFactory::create(Renderer::get()->context(), config);
	}

	void EquirectToolsApplication::write_cubemap_to_png(const std::filesystem::path& out_path)
	{
		Image* const cubemap_image = CACHE_FETCH(Image, EquirectToolsApplication::equirect_cubemap_image);

		const glm::ivec3 cubemap_size = cubemap_image->get_attachment_config().extent;
		const uint32_t mip_count = cubemap_image->get_attachment_config().mip_count;
		const uint32_t layer_count = cubemap_image->get_attachment_config().array_count;
		const uint32_t image_channels = 4;

		const BufferID staging_buffer_id = BufferFactory::create(
			Renderer::get()->context(),
			{
				.name = "equirect_cubemap_transfer_buffer",
				.buffer_size = cubemap_size.x * cubemap_size.y * layer_count * mip_count * image_channels * sizeof(float),
				.type = BufferType::StorageBuffer | BufferType::TransferDestination,
				.memory_usage = MemoryUsageType::OnlyCPU
			},
			false
		);
		Buffer* const staging_buffer = CACHE_FETCH(Buffer, staging_buffer_id);

		// Copy generated cubemap to a staging buffer that we use to do our external image write
		Renderer::get()->context()->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(
			Renderer::get()->context(),
			[this, cubemap_image, cubemap_size, layer_count, mip_count, image_channels, staging_buffer, gfx_context = Renderer::get()->context()](void* command_buffer)
			{
				uint32_t buffer_offset = 0;
				for (uint32_t i = 0; i < layer_count; ++i)
				{
					for (uint32_t j = 0; j < mip_count; ++j)
					{
						cubemap_image->copy_to_buffer(gfx_context, command_buffer, staging_buffer, buffer_offset, j, i);
						buffer_offset += (cubemap_size.x >> j) * (cubemap_size.y >> j) * image_channels * sizeof(float);
					}
				}
			});

		// Use the populated staging buffer to write each cubemap face to a .png
		{
			ScopedGPUBufferMapping buffer_mapping(Renderer::get()->context(), staging_buffer);

			uint32_t buffer_offset = 0;

			// Copy staging buffer to a temporary 8 bit buffer (since the .png format uses 8 bits per component)
			float* float_buffer = (float*)buffer_mapping.mapped_memory;
			uint8_t* pixel_buffer = new uint8_t[cubemap_size.x * cubemap_size.y * image_channels * mip_count * layer_count];
			for (uint32_t i = 0; i < layer_count; ++i)
			{
				for (uint32_t j = 0; j < mip_count; ++j)
				{
					const uint32_t width = (cubemap_size.x >> j);
					const uint32_t height = (cubemap_size.y >> j);

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

			stbi_flip_vertically_on_write(true);

			// Write our temporary 8 bit buffer to a set of .png images
			buffer_offset = 0;
			for (uint32_t i = 0; i < layer_count; ++i)
			{
				for (uint32_t j = 0; j < mip_count; ++j)
				{
					const uint32_t width = (cubemap_size.x >> j);
					const uint32_t height = (cubemap_size.y >> j);
					const uint32_t buffer_size = width * height * image_channels;
					const uint32_t byte_stride = width * image_channels;

					std::ostringstream filename_stream;
					filename_stream << equirect_name << "_cube_mip_" << std::to_string(j) << "_layer_" << std::to_string(i) << ".png";

					std::filesystem::path new_path = out_path / filename_stream.str();

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

	void EquirectToolsApplication::write_irradiance_to_png(const std::filesystem::path& out_path)
	{
		Image* const irradiance_image = CACHE_FETCH(Image, EquirectToolsApplication::irradiance_map_image);

		const glm::ivec3 irradiance_size = irradiance_image->get_attachment_config().extent;
		const uint32_t mip_count = irradiance_image->get_attachment_config().mip_count;
		const uint32_t layer_count = irradiance_image->get_attachment_config().array_count;
		const uint32_t image_channels = 4;

		const BufferID staging_buffer_id = BufferFactory::create(
			Renderer::get()->context(),
			{
				.name = "irradiance_map_transfer_buffer",
				.buffer_size = irradiance_size.x * irradiance_size.y * layer_count * mip_count * image_channels * sizeof(float),
				.type = BufferType::StorageBuffer | BufferType::TransferDestination,
				.memory_usage = MemoryUsageType::OnlyCPU
			},
			false
		);
		Buffer* const staging_buffer = CACHE_FETCH(Buffer, staging_buffer_id);

		// Copy generated irradiance map to a staging buffer that we use to do our external image write
		Renderer::get()->context()->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(
			Renderer::get()->context(),
			[this, irradiance_image, irradiance_size, layer_count, mip_count, image_channels, staging_buffer, gfx_context = Renderer::get()->context()](void* command_buffer)
			{
				uint32_t buffer_offset = 0;
				for (uint32_t i = 0; i < layer_count; ++i)
				{
					for (uint32_t j = 0; j < mip_count; ++j)
					{
						irradiance_image->copy_to_buffer(gfx_context, command_buffer, staging_buffer, buffer_offset, j, i);
						buffer_offset += (irradiance_size.x >> j) * (irradiance_size.y >> j) * image_channels * sizeof(float);
					}
				}
			});

		// Use the populated staging buffer to write each irradiance map face to a .png
		{
			ScopedGPUBufferMapping buffer_mapping(Renderer::get()->context(), staging_buffer);

			uint32_t buffer_offset = 0;

			// Copy staging buffer to a temporary 8 bit buffer (since the .png format uses 8 bits per component)
			float* float_buffer = (float*)buffer_mapping.mapped_memory;
			uint8_t* pixel_buffer = new uint8_t[irradiance_size.x * irradiance_size.y * image_channels * mip_count * layer_count];
			for (uint32_t i = 0; i < layer_count; ++i)
			{
				for (uint32_t j = 0; j < mip_count; ++j)
				{
					const uint32_t width = (irradiance_size.x >> j);
					const uint32_t height = (irradiance_size.y >> j);

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

			stbi_flip_vertically_on_write(true);

			// Write our temporary 8 bit buffer to a set of .png images
			buffer_offset = 0;
			for (uint32_t i = 0; i < layer_count; ++i)
			{
				for (uint32_t j = 0; j < mip_count; ++j)
				{
					const uint32_t width = (irradiance_size.x >> j);
					const uint32_t height = (irradiance_size.y >> j);
					const uint32_t buffer_size = width * height * image_channels;
					const uint32_t byte_stride = width * image_channels;

					std::ostringstream filename_stream;
					filename_stream << equirect_name << "_irradiance_mip_" << std::to_string(j) << "_layer_" << std::to_string(i) << ".png";

					std::filesystem::path new_path = out_path / filename_stream.str();

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

