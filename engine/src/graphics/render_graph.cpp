#include <graphics/render_graph.h>
#include <graphics/render_pass.h>
#include <graphics/graphics_context.h>
#include <graphics/command_queue.h>
#include <graphics/resource/swapchain.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/image.h>
#include <graphics/descriptor.h>
#include <graphics/resource/shader_pipeline_layout.h>
#include <graphics/pipeline_state.h>

#include <unordered_set>

namespace Sunset
{
	void RenderGraph::initialize(GraphicsContext* const gfx_context)
	{
	}

	void RenderGraph::destroy(GraphicsContext* const gfx_context)
	{
		reset(gfx_context);
	}

	void RenderGraph::begin(GraphicsContext* const gfx_context)
	{
		reset(gfx_context);
	}

	Sunset::RGResourceHandle RenderGraph::create_image(class GraphicsContext* const gfx_context, const AttachmentConfig& config)
	{
		RGImageResource* const new_image_resource = image_resource_allocator.get_new();
		new_image_resource->config = config;
		new_image_resource->config.flags |= ImageFlags::Transient;

		const uint32_t index = registry.image_resources.size();
		registry.image_resources.push_back(new_image_resource);
		new_image_resource->handle = create_graph_resource_handle(index, static_cast<RGResourceType>(ResourceType::Image), 1);

		registry.all_resource_handles.push_back(new_image_resource->handle);
		registry.resource_metadata.insert({ new_image_resource->handle, {} });

		return new_image_resource->handle;
	}

	Sunset::RGResourceHandle RenderGraph::register_image(class GraphicsContext* const gfx_context, ImageID image)
	{
		Image* const image_obj = CACHE_FETCH(Image, image);
		RGImageResource* const new_image_resource = image_resource_allocator.get_new();
		new_image_resource->config = image_obj->get_attachment_config();

		const uint32_t index = registry.image_resources.size();
		registry.image_resources.push_back(new_image_resource);
		new_image_resource->handle = create_graph_resource_handle(index, static_cast<RGResourceType>(ResourceType::Image), 1);

		registry.all_resource_handles.push_back(new_image_resource->handle);
		registry.resource_metadata.insert({ new_image_resource->handle, {} });
		registry.resource_metadata[new_image_resource->handle].physical_id = image;
		registry.resource_metadata[new_image_resource->handle].b_is_persistent = true;

		return new_image_resource->handle;
	}

	Sunset::RGResourceHandle RenderGraph::create_buffer(class GraphicsContext* const gfx_context, const BufferConfig& config)
	{
		RGBufferResource* const new_buffer_resource = buffer_resource_allocator.get_new();
		new_buffer_resource->config = config;
		new_buffer_resource->config.type |= BufferType::Transient;

		const uint32_t index = registry.buffer_resources.size();
		registry.buffer_resources.push_back(new_buffer_resource);
		new_buffer_resource->handle = create_graph_resource_handle(index, static_cast<RGResourceType>(ResourceType::Buffer), 1);

		registry.all_resource_handles.push_back(new_buffer_resource->handle);
		registry.resource_metadata.insert({ new_buffer_resource->handle, {} });

		return new_buffer_resource->handle;
	}

	Sunset::RGResourceHandle RenderGraph::register_buffer(class GraphicsContext* const gfx_context, BufferID buffer)
	{
		Buffer* const buffer_obj = CACHE_FETCH(Buffer, buffer);
		RGBufferResource* const new_buffer_resource = buffer_resource_allocator.get_new();
		new_buffer_resource->config = buffer_obj->get_buffer_config();

		const uint32_t index = registry.buffer_resources.size();
		registry.buffer_resources.push_back(new_buffer_resource);
		new_buffer_resource->handle = create_graph_resource_handle(index, static_cast<RGResourceType>(ResourceType::Buffer), 1);

		registry.all_resource_handles.push_back(new_buffer_resource->handle);
		registry.resource_metadata.insert({ new_buffer_resource->handle, {} });
		registry.resource_metadata[new_buffer_resource->handle].physical_id = buffer;
		registry.resource_metadata[new_buffer_resource->handle].b_is_persistent = true;

		return new_buffer_resource->handle;
	}

	Sunset::RGPassHandle RenderGraph::add_pass(class GraphicsContext* const gfx_context, Identity name, RenderPassFlags pass_type, const RGPassParameters& params, std::function<void(RenderGraph&, RGFrameData&, void*)> execution_callback)
	{
		RGPass* const pass = render_pass_allocator.get_new();
		pass->pass_config = { .name = name, .flags = pass_type };
		pass->parameters = params;
		pass->executor = execution_callback;

		const uint32_t index = registry.render_passes.size();
		registry.render_passes.push_back(pass);
		nonculled_passes.push_back(index);

		pass->handle = index;

		update_reference_counts(pass);
		update_resource_param_producers_and_consumers(pass);
		update_present_pass_status(pass);

		return index;
	}

	Sunset::RGPassHandle RenderGraph::add_pass(class GraphicsContext* const gfx_context, Identity name, RenderPassFlags pass_type, std::function<void(RenderGraph&, RGFrameData&, void*)> execution_callback)
	{
		RGPass* const pass = render_pass_allocator.get_new();
		pass->pass_config = { .name = name, .flags = pass_type };
		pass->executor = execution_callback;

		const uint32_t index = registry.render_passes.size();
		registry.render_passes.push_back(pass);
		nonculled_passes.push_back(index);

		pass->handle = index;
		pass->reference_count = 1;

		return index;
	}

	void RenderGraph::update_reference_counts(RGPass* pass)
	{
		pass->reference_count += pass->parameters.outputs.size();
		for (RGResourceHandle resource : pass->parameters.inputs)
		{
			if (auto it = registry.resource_metadata.find(resource); it != registry.resource_metadata.end())
			{
				RGResourceMetadata& metadata = (*it).second;
				metadata.reference_count += 1;
			}
		}
	}

	void RenderGraph::update_resource_param_producers_and_consumers(RGPass* pass)
	{
		for (RGResourceHandle resource : pass->parameters.inputs)
		{
			if (auto it = registry.resource_metadata.find(resource); it != registry.resource_metadata.end())
			{
				RGResourceMetadata& metadata = (*it).second;
				metadata.consumers.push_back(pass->handle);
			}
		}
		for (RGResourceHandle resource : pass->parameters.outputs)
		{
			if (auto it = registry.resource_metadata.find(resource); it != registry.resource_metadata.end())
			{
				RGResourceMetadata& metadata = (*it).second;
				metadata.producers.push_back(pass->handle);
			}
		}
	}

	void RenderGraph::update_present_pass_status(RGPass* pass)
	{
		for (RGResourceHandle resource : pass->parameters.outputs)
		{
			const ResourceType resource_type = static_cast<ResourceType>(get_graph_resource_type(resource));
			const RGResourceIndex resource_index = get_graph_resource_index(resource);
			if (resource_type == ResourceType::Image)
			{
				RGImageResource* const image_resource = registry.image_resources[resource_index];
				const bool b_is_present_resource = (image_resource->config.flags | ImageFlags::Present) != ImageFlags::None;
				if (b_is_present_resource)
				{
					pass->pass_config.b_is_present_pass = true;
					return;
				}
			}
		}
	}

	void RenderGraph::cull_graph_passes(class GraphicsContext* const gfx_context)
	{
		std::unordered_set<RGPassHandle> passes_to_cull;
		std::vector<RGResourceHandle> unused_stack;

		// Lambda to cull a producer pass when it is no longer referenced and pop its inputs onto the unused stack
		auto update_producer_and_subresource_ref_counts = [this, &unused_stack, &passes_to_cull](const std::vector<RGPassHandle> producers)
		{
			for (RGPassHandle pass_handle : producers)
			{
				RGPass* const producer_pass = registry.render_passes[pass_handle];

				// Do not cull present passes as these are usually last and have resources that have not future uses, so would get culled otherwise
				if (producer_pass->pass_config.b_is_present_pass)
				{
					continue;
				}

				--producer_pass->reference_count;

				if (producer_pass->reference_count <= 0)
				{
					producer_pass->reference_count = 0;

					passes_to_cull.insert(pass_handle);

					for (RGResourceHandle resource : producer_pass->parameters.inputs)
					{
						if (auto it = registry.resource_metadata.find(resource); it != registry.resource_metadata.end())
						{
							RGResourceMetadata& metadata = (*it).second;
							metadata.reference_count -= 1;
							if (metadata.reference_count == 0)
							{
								unused_stack.push_back(resource);
							}
						}
					}
				}
			}
		};

		// Go through all resources and push any resources that are not referenced onto the unused stack
		for (RGResourceHandle resource : registry.all_resource_handles)
		{
			if (auto it = registry.resource_metadata.find(resource); it != registry.resource_metadata.end())
			{
				if ((*it).second.reference_count == 0)
				{
					unused_stack.push_back(resource);
				}
			}
		}

		// Keep processing unused resources and updating their producer pass ref counts
		while (!unused_stack.empty())
		{
			RGResourceHandle unused_resource = unused_stack.back();
			unused_stack.pop_back();

			if (auto it = registry.resource_metadata.find(unused_resource); it != registry.resource_metadata.end())
			{
				update_producer_and_subresource_ref_counts((*it).second.producers);
			}
		}

		// Take all the culled passes and remove them from our primary nonculled_passes array.
		// This may constantly shuffle array items if a lot of passes are culled so maybe think about
		// using a flag instead to indicate if a pass is active or not.
		for (int i = nonculled_passes.size() - 1; i >= 0; --i)
		{
			RGPassHandle pass = nonculled_passes[i];
			if (passes_to_cull.find(pass) != passes_to_cull.end())
			{
				nonculled_passes.erase(nonculled_passes.begin() + i);
			}
		}
	}

	void RenderGraph::compute_resource_first_and_last_users(class GraphicsContext* const gfx_context)
	{
		for (RGResourceHandle resource : registry.all_resource_handles)
		{
			if (auto it = registry.resource_metadata.find(resource); it != registry.resource_metadata.end())
			{
				RGResourceMetadata& metadata = (*it).second;
				if (metadata.reference_count == 0)
				{
					continue;
				}

				metadata.first_user = std::numeric_limits<uint32_t>::max();
				metadata.last_user = std::numeric_limits<uint32_t>::min();
				for (RGPassHandle pass : metadata.producers)
				{
					if (pass < metadata.first_user)
					{
						metadata.first_user = pass;
					}
					if (pass > metadata.last_user)
					{
						metadata.last_user = pass;
					}
				}
				for (RGPassHandle pass : metadata.consumers)
				{
					if (pass < metadata.first_user)
					{
						metadata.first_user = pass;
					}
					if (pass > metadata.last_user)
					{
						metadata.last_user = pass;
					}
				}
			}
		}
	}

	void RenderGraph::create_dummy_pipeline_layout(class GraphicsContext* const gfx_context)
	{
		if (dummy_pipeline_layout == 0)
		{
			// Used to bind top level descriptor sets early so individual passes only need to worry about more granular descriptor set binds.
			// Descriptor sets will not be rebound if the corresponding descriptor set slot layout does not change on subsequent pipeline state changes.
			dummy_pipeline_layout = ShaderPipelineLayoutFactory::create(
				gfx_context,
				{},
				{
					global_descriptor_datas[0].descriptor_layout
				}
			);
		}
	}

	void RenderGraph::compile(class GraphicsContext* const gfx_context, class Swapchain* const swapchain)
	{
		cull_graph_passes(gfx_context);
		compute_resource_first_and_last_users(gfx_context);
		// TODO: Compute async wait points and resource barriers for non-culled resources
		// TODO: Not entirely sure how to do this yet (with VMA?) but walk resource lifetimes to account for how much GPU memory should be allocated up front for aliasing
		create_dummy_pipeline_layout(gfx_context);
	}

	void RenderGraph::submit(GraphicsContext* const gfx_context, class Swapchain* const swapchain)
	{
		compile(gfx_context, swapchain);

		// TODO: switch the queue based on the pass type
		void* cmd_buffer = gfx_context->get_command_queue(DeviceQueueType::Graphics)->begin_one_time_buffer_record(gfx_context);

		bind_global_descriptors(gfx_context, cmd_buffer);

		{
			RGFrameData frame_data
			{
				.gfx_context = gfx_context
			};

			for (RGPassHandle pass_handle : nonculled_passes)
			{
				execute_pass(gfx_context, swapchain, registry.render_passes[pass_handle], frame_data, cmd_buffer);
			}
		}

		// TODO: switch the queue based on the pass type
		gfx_context->get_command_queue(DeviceQueueType::Graphics)->end_one_time_buffer_record(gfx_context);
		// TODO: switch the queue based on the pass type
		gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit(gfx_context);
	}

	void RenderGraph::inject_global_descriptors(class GraphicsContext* const gfx_context, uint16_t buffered_frame_index, const std::initializer_list<DescriptorBuildData>& descriptor_build_datas)
	{
		DescriptorHelpers::inject_descriptors(gfx_context, global_descriptor_datas[buffered_frame_index], descriptor_build_datas);
	}

	size_t RenderGraph::get_physical_resource(RGResourceHandle resource)
	{
		if (registry.resource_metadata.find(resource) != registry.resource_metadata.end())
		{
			return registry.resource_metadata[resource].physical_id;
		}
		return 0;
	}

	void RenderGraph::reset(class GraphicsContext* const gfx_context)
	{
		free_physical_resources(gfx_context);

		nonculled_passes.clear();

		registry.all_resource_handles.clear();
		registry.buffer_resources.clear();
		registry.image_resources.clear();
		registry.render_passes.clear();
		registry.resource_metadata.clear();

		image_resource_allocator.reset();
		buffer_resource_allocator.reset();
		render_pass_allocator.reset();
	}

	void RenderGraph::execute_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGPass* pass, RGFrameData& frame_data, void* command_buffer)
	{
		assert(pass != nullptr);

		if ((pass->pass_config.flags & RenderPassFlags::GraphLocal) != RenderPassFlags::None)
		{
			pass->executor(*this, frame_data, command_buffer);
		}
		else
		{
			setup_physical_pass_and_resources(gfx_context, swapchain, pass->handle, command_buffer);

			if ((pass->pass_config.flags & RenderPassFlags::Compute) != RenderPassFlags::None)
			{
				pass->executor(*this, frame_data, command_buffer);
			}
			else
			{
				RenderPass* const physical_pass = CACHE_FETCH(RenderPass, pass->physical_id);
				assert(physical_pass != nullptr);

				frame_data.current_pass = pass->physical_id;

				physical_pass->begin_pass(gfx_context, pass->pass_config.b_is_present_pass ? swapchain->get_current_image_index() : 0, command_buffer);
				pass->executor(*this, frame_data, command_buffer);
				physical_pass->end_pass(gfx_context, command_buffer);
			}

			update_transient_resources(gfx_context, pass);
		}
	}

	void RenderGraph::setup_physical_pass_and_resources(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGPassHandle pass, void* command_buffer)
	{
		RGPass* const graph_pass = registry.render_passes[pass];

		const bool b_is_graphics_pass = (graph_pass->pass_config.flags & RenderPassFlags::Main) != RenderPassFlags::None;

		// Setup resource used by the render pass. If they are output/input attachments we will handle those here as well but cache those off
		// as those necessarily need caching along with the render passes.
		for (RGResourceHandle input_resource : graph_pass->parameters.inputs)
		{
			setup_physical_resource(gfx_context, swapchain, input_resource, b_is_graphics_pass, true);
			if (b_is_graphics_pass)
			{
				tie_resource_to_pass_config_attachments(gfx_context, input_resource, graph_pass, true);
			}
		}
		for (RGResourceHandle output_resource : graph_pass->parameters.outputs)
		{
			setup_physical_resource(gfx_context, swapchain, output_resource, b_is_graphics_pass, false);
			if (b_is_graphics_pass)
			{
				tie_resource_to_pass_config_attachments(gfx_context, output_resource, graph_pass, false);
			}
		}

		// Create our physical render pass and cache it if necessary
		graph_pass->physical_id = RenderPassFactory::create_default(gfx_context, swapchain, graph_pass->pass_config, true);

		// Setup render pass pipeline state if render pass-level shader stages were provided. Otherwise, pipeline state should be handled
		// and bound by render pass callbacks. Also sparsely bind our descriptors and push constants.
		setup_pass_pipeline_state_if_necessary(gfx_context, graph_pass, command_buffer);
		setup_pass_descriptors(gfx_context, graph_pass, command_buffer);
		bind_pass_descriptors(gfx_context, graph_pass, command_buffer);
		push_pass_constants(gfx_context, graph_pass, command_buffer);
	}

	void RenderGraph::setup_physical_resource(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGResourceHandle resource, bool b_is_graphics_pass, bool b_is_input_resource)
	{
		const ResourceType resource_type = static_cast<ResourceType>(get_graph_resource_type(resource));
		const RGResourceIndex resource_index = get_graph_resource_index(resource);
		if (resource_type == ResourceType::Buffer)
		{
			RGBufferResource* const buffer_resource = registry.buffer_resources[resource_index];
			// We check for an empty ID first because registered external resources would have already had this field populated
			if (registry.resource_metadata[resource].physical_id == 0)
			{
				registry.resource_metadata[resource].physical_id = BufferFactory::create(gfx_context, buffer_resource->config, false);
			}
		}
		else if (resource_type == ResourceType::Image)
		{
			RGImageResource* const image_resource = registry.image_resources[resource_index];
			const bool b_is_local_load = (image_resource->config.flags & ImageFlags::LocalLoad) != ImageFlags::None;
			const bool b_is_persistent = b_is_graphics_pass && (!b_is_input_resource || b_is_local_load);
			// We check for an empty ID first because registered external resources would have already had this field populated
			if (registry.resource_metadata[resource].physical_id == 0)
			{
				registry.resource_metadata[resource].physical_id = ImageFactory::create(gfx_context, image_resource->config, b_is_persistent);
				registry.resource_metadata[resource].b_is_persistent = b_is_persistent;
			}
		}
	}

	void RenderGraph::tie_resource_to_pass_config_attachments(class GraphicsContext* const gfx_context, RGResourceHandle resource, RGPass* pass, bool b_is_input_resource)
	{
		const ResourceType resource_type = static_cast<ResourceType>(get_graph_resource_type(resource));
		const RGResourceIndex resource_index = get_graph_resource_index(resource);
		if (resource_type == ResourceType::Image)
		{
			RGImageResource* const image_resource = registry.image_resources[resource_index];
			const bool b_is_local_load = (image_resource->config.flags & ImageFlags::LocalLoad) != ImageFlags::None;
			if (!b_is_input_resource || b_is_local_load)
			{
				pass->pass_config.attachments.push_back(registry.resource_metadata[resource].physical_id);
			}
		}
	}

	void RenderGraph::setup_pass_pipeline_state_if_necessary(class GraphicsContext* const gfx_context, RGPass* pass, void* command_buffer)
	{
		RGShaderDataSetup& shader_setup = pass->parameters.shader_setup;
		if (!shader_setup.pipeline_shaders.empty())
		{
			const bool b_is_compute_pass = (pass->pass_config.flags & RenderPassFlags::Compute) != RenderPassFlags::None;
			if (b_is_compute_pass)
			{
				PipelineComputeStateBuilder state_builder = PipelineComputeStateBuilder::create()
					.clear_shader_stages()
					.value();

				if (shader_setup.push_constant_data.data != nullptr)
				{
					state_builder.set_push_constants(shader_setup.push_constant_data);
				}

				for (const auto& shader_stage : shader_setup.pipeline_shaders)
				{
					state_builder.set_shader_stage(shader_stage.second);
				}

				state_builder.derive_shader_layout();

				pass->pipeline_state_id = state_builder.finish();
			}
			else
			{
				PipelineGraphicsStateBuilder state_builder = PipelineGraphicsStateBuilder::create()
					.clear_shader_stages()
					.set_pass(pass->physical_id)
					.value();

				if (shader_setup.push_constant_data.data != nullptr)
				{
					state_builder.set_push_constants(shader_setup.push_constant_data);
				}

				for (const auto& shader_stage : shader_setup.pipeline_shaders)
				{
					state_builder.set_shader_stage(shader_stage.first, shader_stage.second);
				}

				state_builder.derive_shader_layout();

				pass->pipeline_state_id = state_builder.finish();
			}
		}

		if (pass->pipeline_state_id != 0)
		{
			CACHE_FETCH(PipelineState, pass->pipeline_state_id)->bind(gfx_context, command_buffer);
		}
	}

	void RenderGraph::setup_pass_descriptors(class GraphicsContext* const gfx_context, RGPass* pass, void* command_buffer)
	{
		for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			if (registry.pass_descriptor_cache.find(pass->pass_config.name) == registry.pass_descriptor_cache.end())
			{
				registry.pass_descriptor_cache[pass->pass_config.name][i] = DescriptorData();
			}

			DescriptorData& pass_descriptor_data = registry.pass_descriptor_cache[pass->pass_config.name][i];
			{
				std::vector<DescriptorBuildData> descriptor_build_datas;
				// TODO: Figure out how to skip shader setup declarations if the RG pass was created with shader stages,
				//		 and so has it's own pipeline state.
				for (uint32_t j = 0; j < pass->parameters.shader_setup.declarations.size(); ++j)
				{
					const RGShaderDescriptorDeclaration& decl = pass->parameters.shader_setup.declarations[j];

					if (decl.type == DescriptorType::Image)
					{
						DescriptorBuildData& build_data = descriptor_build_datas.emplace_back();
						build_data.binding = j;
						build_data.image = registry.resource_metadata[pass->parameters.inputs[j]].physical_id;
						build_data.count = decl.count;
						build_data.type = decl.type;
						build_data.shader_stages = decl.shader_stages;
						build_data.b_supports_bindless = decl.b_supports_bindless;
					}
					else
					{
						DescriptorBuildData& build_data = descriptor_build_datas.emplace_back();
						build_data.binding = j;
						build_data.buffer = registry.resource_metadata[pass->parameters.inputs[j]].physical_id;
						build_data.buffer_offset = 0;
						build_data.buffer_range = CACHE_FETCH(Buffer, build_data.buffer)->get_size(); // Do we want to explicitly provide this? (i.e. dynamic uniform buffers)
						build_data.count = decl.count;
						build_data.type = decl.type;
						build_data.shader_stages = decl.shader_stages;
						build_data.b_supports_bindless = decl.b_supports_bindless;
					}
				}

				DescriptorHelpers::inject_descriptors(gfx_context, pass_descriptor_data, descriptor_build_datas);
			}

			if (pass->pipeline_state_id != 0)
			{
				pass_descriptor_data.pipeline_layout = CACHE_FETCH(PipelineState, pass->pipeline_state_id)->get_state_data().layout;
			}
			else
			{
				pass_descriptor_data.pipeline_layout = ShaderPipelineLayoutFactory::create(
				gfx_context,
				{},
					{
						global_descriptor_datas[i].descriptor_layout,
						pass_descriptor_data.descriptor_layout
					}
				);
			}
		}
	}

	void RenderGraph::bind_global_descriptors(class GraphicsContext* const gfx_context, void* command_buffer)
	{
		DescriptorData& descriptor_data = global_descriptor_datas[gfx_context->get_buffered_frame_number()];
		if (descriptor_data.descriptor_set != nullptr)
		{
			descriptor_data.descriptor_set->bind(gfx_context, command_buffer, dummy_pipeline_layout, PipelineStateType::Graphics, descriptor_data.dynamic_buffer_offsets, static_cast<uint16_t>(DescriptorSetType::Global));
		}
	}

	void RenderGraph::bind_pass_descriptors(class GraphicsContext* const gfx_context, RGPass* pass, void* command_buffer)
	{
		const uint16_t buffered_frame_index = gfx_context->get_buffered_frame_number();

		DescriptorData& pass_descriptor_data = registry.pass_descriptor_cache[pass->pass_config.name][buffered_frame_index];

		if (pass_descriptor_data.descriptor_set != nullptr)
		{
			const bool b_is_compute_pass = (pass->pass_config.flags & RenderPassFlags::Compute) != RenderPassFlags::None;
			pass_descriptor_data.descriptor_set->bind(
				gfx_context,
				command_buffer,
				pass_descriptor_data.pipeline_layout,
				b_is_compute_pass ? PipelineStateType::Compute : PipelineStateType::Graphics,
				pass_descriptor_data.dynamic_buffer_offsets,
				static_cast<uint16_t>(DescriptorSetType::Pass)
			);
		}
	}

	void RenderGraph::push_pass_constants(class GraphicsContext* const gfx_context, RGPass* pass, void* command_buffer)
	{
		if (pass->parameters.shader_setup.push_constant_data.is_valid())
		{
			gfx_context->push_constants(command_buffer, pass->pipeline_state_id, pass->parameters.shader_setup.push_constant_data);
		}
	}

	void RenderGraph::update_transient_resources(class GraphicsContext* const gfx_context, RGPass* pass)
	{
		// TODO: Look into aliasing memory for expired resources as opposed to just flat out deleting them (though that should also be done)
		for (RGResourceHandle input_resource : pass->parameters.inputs)
		{
			RGResourceMetadata& resource_meta = registry.resource_metadata[input_resource];
			if (resource_meta.physical_id != 0 && resource_meta.last_user == pass->handle && !resource_meta.b_is_persistent)
			{
				// TODO: Aliasing
			}
		}
		for (RGResourceHandle output_resource : pass->parameters.outputs)
		{
			RGResourceMetadata& resource_meta = registry.resource_metadata[output_resource];
			if (resource_meta.physical_id != 0 && resource_meta.last_user == pass->handle && !resource_meta.b_is_persistent)
			{
				// TODO: Aliasing
			}
		}
	}

	void RenderGraph::free_physical_resources(class GraphicsContext* const gfx_context)
	{
		// TODO: Look into aliasing memory for expired resources as opposed to just flat out deleting them (though that should also be done)
		for (RGResourceHandle resource : registry.all_resource_handles)
		{
			RGResourceMetadata& resource_meta = registry.resource_metadata[resource];
			if (resource_meta.physical_id != 0 && !resource_meta.b_is_persistent)
			{
				const ResourceType resource_type = static_cast<ResourceType>(get_graph_resource_type(resource));
				if (resource_type == ResourceType::Image)
				{
					CACHE_DELETE(Image, resource_meta.physical_id, gfx_context);
				}
				else if (resource_type == ResourceType::Buffer)
				{
					CACHE_DELETE(Buffer, resource_meta.physical_id, gfx_context);
				}
				resource_meta.physical_id = 0;
			}
		}
	}
}
