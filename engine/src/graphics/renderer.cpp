#include <graphics/renderer.h>
#include <window/window.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/swapchain.h>
#include <graphics/command_queue.h>
#include <graphics/pipeline_state.h>
#include <graphics/render_pass.h>
#include <graphics/resource/shader_pipeline_layout.h>

namespace Sunset
{
	void Renderer::initialize(Window* const window)
	{
		graphics_context = GraphicsContextFactory::create(window);
		swapchain = SwapchainFactory::create(graphics_context);
		command_queue = GraphicsCommandQueueFactory::create(graphics_context);

		PipelineState* pipeline_state_colored = PipelineStateBuilder::create(graphics_context)
			.add_viewport(0.0f, 0.0f, static_cast<float>(window->get_extent().x), static_cast<float>(window->get_extent().y), 0.0f, 1.0f)
			.add_scissor(0, 0, window->get_extent().x, window->get_extent().y)
			.set_shader_layout(ShaderPipelineLayoutFactory::create(graphics_context))
			.set_shader_stage(PipelineShaderStageType::Vertex, "../../shaders/basic_colored.vert.spv")
			.set_shader_stage(PipelineShaderStageType::Fragment, "../../shaders/basic_colored.frag.spv")
			.set_primitive_topology_type(PipelinePrimitiveTopologyType::TriangleList)
			.set_rasterizer_state(PipelineRasterizerPolygonMode::Fill, 1.0f, PipelineRasterizerCullMode::None)
			.set_multisample_count(1)
			.get_state();

		PipelineState* pipeline_state_basic = PipelineStateBuilder::create(graphics_context)
			.add_viewport(0.0f, 0.0f, static_cast<float>(window->get_extent().x), static_cast<float>(window->get_extent().y), 0.0f, 1.0f)
			.add_scissor(0, 0, window->get_extent().x, window->get_extent().y)
			.set_shader_layout(ShaderPipelineLayoutFactory::create(graphics_context))
			.set_shader_stage(PipelineShaderStageType::Vertex, "../../shaders/basic.vert.spv")
			.set_shader_stage(PipelineShaderStageType::Fragment, "../../shaders/basic.frag.spv")
			.set_primitive_topology_type(PipelinePrimitiveTopologyType::TriangleList)
			.set_rasterizer_state(PipelineRasterizerPolygonMode::Fill, 1.0f, PipelineRasterizerCullMode::None)
			.set_multisample_count(1)
			.get_state();

		render_pass = RenderPassFactory::create_default(graphics_context, swapchain, { pipeline_state_basic, pipeline_state_colored });
	}

	void Renderer::draw()
	{
		graphics_context->wait_for_gpu();

		swapchain->request_next_image(graphics_context);

		void* buffer = command_queue->begin_one_time_buffer_record(graphics_context);

		render_pass->begin_pass(graphics_context, swapchain, buffer);
		render_pass->draw(graphics_context, buffer);
		render_pass->end_pass(graphics_context, swapchain, buffer);

		command_queue->end_one_time_buffer_record(graphics_context);

		command_queue->submit(graphics_context);

		swapchain->present(graphics_context, command_queue);

		graphics_context->advance_frame();
	}

	void Renderer::destroy()
	{
		graphics_context->wait_for_gpu();
		render_pass->destroy(graphics_context);
		command_queue->destroy(graphics_context);
		swapchain->destroy(graphics_context);
		graphics_context->destroy();
	}
}
