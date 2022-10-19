#include <graphics/renderer.h>
#include <window/window.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/mesh.h>
#include <graphics/resource/swapchain.h>
#include <graphics/pipeline_state.h>
#include <graphics/render_pass.h>
#include <graphics/resource/shader_pipeline_layout.h>

namespace Sunset
{
	void Renderer::initialize(Window* const window)
	{
		graphics_context = GraphicsContextFactory::create(window);
		graphics_context->set_buffer_allocator(BufferAllocatorFactory::create(graphics_context.get()));

		swapchain = SwapchainFactory::create(graphics_context.get());
		command_queue = GraphicsCommandQueueFactory::create(graphics_context.get());

		PipelineStateID pipeline_state_colored = PipelineStateBuilder::create(graphics_context.get())
			.add_viewport(0.0f, 0.0f, static_cast<float>(window->get_extent().x), static_cast<float>(window->get_extent().y), 0.0f, 1.0f)
			.add_scissor(0, 0, window->get_extent().x, window->get_extent().y)
			.set_shader_layout(ShaderPipelineLayoutFactory::create(graphics_context.get()))
			.set_shader_stage(PipelineShaderStageType::Vertex, "../../shaders/basic_colored.vert.spv")
			.set_shader_stage(PipelineShaderStageType::Fragment, "../../shaders/basic_colored.frag.spv")
			.set_vertex_input_description(Vertex::get_description())
			.set_primitive_topology_type(PipelinePrimitiveTopologyType::TriangleList)
			.set_rasterizer_state(PipelineRasterizerPolygonMode::Fill, 1.0f, PipelineRasterizerCullMode::None)
			.set_multisample_count(1)
			.finish();

		PipelineStateID pipeline_state_basic = PipelineStateBuilder::create(graphics_context.get())
			.add_viewport(0.0f, 0.0f, static_cast<float>(window->get_extent().x), static_cast<float>(window->get_extent().y), 0.0f, 1.0f)
			.add_scissor(0, 0, window->get_extent().x, window->get_extent().y)
			.set_shader_layout(ShaderPipelineLayoutFactory::create(graphics_context.get()))
			.set_shader_stage(PipelineShaderStageType::Vertex, "../../shaders/basic.vert.spv")
			.set_shader_stage(PipelineShaderStageType::Fragment, "../../shaders/basic.frag.spv")
			.set_vertex_input_description(Vertex::get_description())
			.set_primitive_topology_type(PipelinePrimitiveTopologyType::TriangleList)
			.set_rasterizer_state(PipelineRasterizerPolygonMode::Fill, 1.0f, PipelineRasterizerCullMode::None)
			.set_multisample_count(1)
			.finish();

		render_pass = RenderPassFactory::create_default(graphics_context.get(), swapchain, { pipeline_state_basic, pipeline_state_colored });
	}

	void Renderer::draw()
	{
		graphics_context->wait_for_gpu();

		swapchain->request_next_image(graphics_context.get());

		void* buffer = command_queue->begin_one_time_buffer_record(graphics_context.get());

		render_pass->begin_pass(graphics_context.get(), swapchain, buffer);
		render_pass->draw(graphics_context.get(), buffer);
		render_pass->end_pass(graphics_context.get(), swapchain, buffer);

		command_queue->end_one_time_buffer_record(graphics_context.get());

		command_queue->submit(graphics_context.get());

		swapchain->present(graphics_context.get(), command_queue.get());

		graphics_context->advance_frame();
	}

	void Renderer::destroy()
	{
		graphics_context->wait_for_gpu();
		render_pass->destroy(graphics_context.get());
		command_queue->destroy(graphics_context.get());
		swapchain->destroy(graphics_context.get());
		graphics_context->destroy();
	}
}
