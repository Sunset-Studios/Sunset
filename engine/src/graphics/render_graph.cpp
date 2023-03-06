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
		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			reset(gfx_context, i);
		}
	}

	void RenderGraph::begin(GraphicsContext* const gfx_context)
	{
		reset(gfx_context, gfx_context->get_buffered_frame_number());
	}

	Sunset::RGResourceHandle RenderGraph::create_image(class GraphicsContext* const gfx_context, const AttachmentConfig& config)
	{
		RGImageResource* const new_image_resource = image_resource_allocator.get_new();
		new_image_resource->config = config;
		new_image_resource->config.flags |= ImageFlags::Transient;
		new_image_resource->config.name.computed_hash += gfx_context->get_buffered_frame_number();

		const uint32_t index = current_registry->image_resources.size();
		current_registry->image_resources.push_back(new_image_resource);
		new_image_resource->handle = create_graph_resource_handle(index, static_cast<RGResourceType>(ResourceType::Image), 1);

		current_registry->all_resource_handles.push_back(new_image_resource->handle);
		current_registry->resource_metadata.insert({ new_image_resource->handle, {} });

		return new_image_resource->handle;
	}

	Sunset::RGResourceHandle RenderGraph::register_image(class GraphicsContext* const gfx_context, ImageID image)
	{
		Image* const image_obj = CACHE_FETCH(Image, image);
		RGImageResource* const new_image_resource = image_resource_allocator.get_new();
		new_image_resource->config = image_obj->get_attachment_config();

		const uint32_t index = current_registry->image_resources.size();
		current_registry->image_resources.push_back(new_image_resource);
		new_image_resource->handle = create_graph_resource_handle(index, static_cast<RGResourceType>(ResourceType::Image), 1);

		current_registry->all_resource_handles.push_back(new_image_resource->handle);
		current_registry->resource_metadata.insert({ new_image_resource->handle, {} });
		current_registry->resource_metadata[new_image_resource->handle].physical_id = image;
		current_registry->resource_metadata[new_image_resource->handle].b_is_persistent = true;

		return new_image_resource->handle;
	}

	Sunset::RGResourceHandle RenderGraph::create_buffer(class GraphicsContext* const gfx_context, const BufferConfig& config)
	{
		RGBufferResource* const new_buffer_resource = buffer_resource_allocator.get_new();
		new_buffer_resource->config = config;
		new_buffer_resource->config.type |= BufferType::Transient;
		new_buffer_resource->config.name.computed_hash += gfx_context->get_buffered_frame_number();

		const uint32_t index = current_registry->buffer_resources.size();
		current_registry->buffer_resources.push_back(new_buffer_resource);
		new_buffer_resource->handle = create_graph_resource_handle(index, static_cast<RGResourceType>(ResourceType::Buffer), 1);

		current_registry->all_resource_handles.push_back(new_buffer_resource->handle);
		current_registry->resource_metadata.insert({ new_buffer_resource->handle, {} });

		return new_buffer_resource->handle;
	}

	Sunset::RGResourceHandle RenderGraph::register_buffer(class GraphicsContext* const gfx_context, BufferID buffer)
	{
		Buffer* const buffer_obj = CACHE_FETCH(Buffer, buffer);
		RGBufferResource* const new_buffer_resource = buffer_resource_allocator.get_new();
		new_buffer_resource->config = buffer_obj->get_buffer_config();

		const uint32_t index = current_registry->buffer_resources.size();
		current_registry->buffer_resources.push_back(new_buffer_resource);
		new_buffer_resource->handle = create_graph_resource_handle(index, static_cast<RGResourceType>(ResourceType::Buffer), 1);

		current_registry->all_resource_handles.push_back(new_buffer_resource->handle);
		current_registry->resource_metadata.insert({ new_buffer_resource->handle, {} });
		current_registry->resource_metadata[new_buffer_resource->handle].physical_id = buffer;
		current_registry->resource_metadata[new_buffer_resource->handle].b_is_persistent = true;

		return new_buffer_resource->handle;
	}

	Sunset::RGPassHandle RenderGraph::add_pass(class GraphicsContext* const gfx_context, Identity name, RenderPassFlags pass_type, const RGPassParameters& params, std::function<void(RenderGraph&, RGFrameData&, void*)> execution_callback)
	{
		RGPass* const pass = render_pass_allocator.get_new();
		pass->pass_config = { .name = name, .flags = pass_type };
		pass->parameters = params;
		pass->executor = execution_callback;

		const uint32_t index = current_registry->render_passes.size();
		current_registry->render_passes.push_back(pass);
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

		const uint32_t index = current_registry->render_passes.size();
		current_registry->render_passes.push_back(pass);
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
			if (auto it = current_registry->resource_metadata.find(resource); it != current_registry->resource_metadata.end())
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
			if (auto it = current_registry->resource_metadata.find(resource); it != current_registry->resource_metadata.end())
			{
				RGResourceMetadata& metadata = (*it).second;
				metadata.consumers.push_back(pass->handle);
			}
		}
		for (RGResourceHandle resource : pass->parameters.outputs)
		{
			if (auto it = current_registry->resource_metadata.find(resource); it != current_registry->resource_metadata.end())
			{
				RGResourceMetadata& metadata = (*it).second;
				metadata.producers.push_back(pass->handle);
			}
		}
	}

	void RenderGraph::update_present_pass_status(RGPass* pass)
	{
		pass->pass_config.b_is_present_pass = (pass->pass_config.flags & RenderPassFlags::Present) != RenderPassFlags::None;
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
				RGPass* const producer_pass = current_registry->render_passes[pass_handle];

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
						if (auto it = current_registry->resource_metadata.find(resource); it != current_registry->resource_metadata.end())
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
		for (RGResourceHandle resource : current_registry->all_resource_handles)
		{
			if (auto it = current_registry->resource_metadata.find(resource); it != current_registry->resource_metadata.end())
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

			if (auto it = current_registry->resource_metadata.find(unused_resource); it != current_registry->resource_metadata.end())
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
		for (RGResourceHandle resource : current_registry->all_resource_handles)
		{
			if (auto it = current_registry->resource_metadata.find(resource); it != current_registry->resource_metadata.end())
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

	void RenderGraph::compile(class GraphicsContext* const gfx_context, class Swapchain* const swapchain)
	{
		cull_graph_passes(gfx_context);
		compute_resource_first_and_last_users(gfx_context);
		// TODO: Compute async wait points and resource barriers for non-culled resources
		// TODO: Not entirely sure how to do this yet (with VMA?) but walk resource lifetimes to account for how much GPU memory should be allocated up front for aliasing
	}

	void RenderGraph::submit(GraphicsContext* const gfx_context, class Swapchain* const swapchain)
	{
		compile(gfx_context, swapchain);

		if (nonculled_passes.empty())
		{
			return;
		}

		// TODO: switch the queue based on the pass type
		void* cmd_buffer = gfx_context->get_command_queue(DeviceQueueType::Graphics)->begin_one_time_buffer_record(gfx_context);

		{
			RGFrameData frame_data
			{
				.gfx_context = gfx_context,
				.resource_deletion_queue = &current_registry->resource_deletion_queue
			};

			for (RGPassHandle pass_handle : nonculled_passes)
			{
				execute_pass(gfx_context, swapchain, current_registry->render_passes[pass_handle], frame_data, cmd_buffer);
			}
		}

		// TODO: switch the queue based on the pass type
		gfx_context->get_command_queue(DeviceQueueType::Graphics)->end_one_time_buffer_record(gfx_context);
		// TODO: switch the queue based on the pass type
		gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit(gfx_context);
	}

	void RenderGraph::queue_global_descriptor_writes(class GraphicsContext* const gfx_context, uint32_t buffered_frame, const std::initializer_list<DescriptorBufferDesc>& buffers)
	{
		assert(buffered_frame >= 0 && buffered_frame < MAX_BUFFERED_FRAMES);
		queued_buffer_global_writes[buffered_frame] = buffers;
	}

	size_t RenderGraph::get_physical_resource(RGResourceHandle resource)
	{
		if (current_registry->resource_metadata.find(resource) != current_registry->resource_metadata.end())
		{
			return current_registry->resource_metadata[resource].physical_id;
		}
		return 0;
	}

	void RenderGraph::reset(class GraphicsContext* const gfx_context, uint32_t current_buffered_frame)
	{
		current_registry = &registries[current_buffered_frame];

		free_physical_resources(gfx_context);

		nonculled_passes.clear();

		current_registry->all_resource_handles.clear();
		current_registry->buffer_resources.clear();
		current_registry->image_resources.clear();
		current_registry->render_passes.clear();
		current_registry->resource_metadata.clear();

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

			{
				DescriptorDataList& descriptor_data_list = pass_cache.descriptors[pass->pass_config.name];
				frame_data.pass_descriptor_set = descriptor_data_list.descriptor_sets.empty()
					? nullptr
					: descriptor_data_list.descriptor_sets[static_cast<uint32_t>(DescriptorSetType::Pass)];
			}

			frame_data.pass_pipeline_state = pass->pipeline_state_id;

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
		RGPass* const graph_pass = current_registry->render_passes[pass];

		const bool b_is_graphics_pass = (graph_pass->pass_config.flags & RenderPassFlags::Graphics) != RenderPassFlags::None;

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

		// Create our physical render pass (is internally cached if necessary using a hash based on the pass config)
		graph_pass->physical_id = RenderPassFactory::create_default(gfx_context, swapchain, graph_pass->pass_config, true);

		// Setup render pass pipeline state if render pass-level shader stages were provided. Otherwise, pipeline state should be handled
		// and bound by render pass callbacks. Also sparsely bind our descriptors and push constants.
		setup_pass_pipeline_state(gfx_context, graph_pass, command_buffer);
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
			RGBufferResource* const buffer_resource = current_registry->buffer_resources[resource_index];
			// We check for an empty ID first because registered external resources would have already had this field populated
			if (current_registry->resource_metadata[resource].physical_id == 0)
			{
				current_registry->resource_metadata[resource].physical_id = BufferFactory::create(gfx_context, buffer_resource->config, false);
				if (!current_registry->resource_metadata[resource].b_is_persistent)
				{
					current_registry->resource_deletion_queue.push_execution([gfx_context, physical_id = current_registry->resource_metadata[resource].physical_id]()
						{
							CACHE_DELETE(Buffer, physical_id, gfx_context);
						}
					);
				}
			}
		}
		else if (resource_type == ResourceType::Image)
		{
			RGImageResource* const image_resource = current_registry->image_resources[resource_index];
			const bool b_is_local_load = (image_resource->config.flags & ImageFlags::LocalLoad) != ImageFlags::None;
			const bool b_is_persistent = b_is_graphics_pass && (!b_is_input_resource || b_is_local_load);
			// We check for an empty ID first because registered external resources would have already had this field populated
			if (current_registry->resource_metadata[resource].physical_id == 0)
			{
				current_registry->resource_metadata[resource].physical_id = ImageFactory::create(gfx_context, image_resource->config, b_is_persistent);
				current_registry->resource_metadata[resource].b_is_persistent = b_is_persistent;
				if (!current_registry->resource_metadata[resource].b_is_persistent)
				{
					current_registry->resource_deletion_queue.push_execution([gfx_context, physical_id = current_registry->resource_metadata[resource].physical_id]()
						{
							CACHE_DELETE(Image, physical_id, gfx_context);
						}
					);
				}
			}
		}
	}

	void RenderGraph::tie_resource_to_pass_config_attachments(class GraphicsContext* const gfx_context, RGResourceHandle resource, RGPass* pass, bool b_is_input_resource)
	{
		const ResourceType resource_type = static_cast<ResourceType>(get_graph_resource_type(resource));
		const RGResourceIndex resource_index = get_graph_resource_index(resource);
		if (resource_type == ResourceType::Image)
		{
			RGImageResource* const image_resource = current_registry->image_resources[resource_index];
			const bool b_is_local_load = (image_resource->config.flags & ImageFlags::LocalLoad) != ImageFlags::None;
			if (!b_is_input_resource || b_is_local_load)
			{
				pass->pass_config.attachments.push_back(current_registry->resource_metadata[resource].physical_id);
			}
		}
	}

	void RenderGraph::setup_pass_pipeline_state(class GraphicsContext* const gfx_context, RGPass* pass, void* command_buffer)
	{
		// We don't "need" the additional pipeline state caching here since internally pipeline states are already cached
		// based on the builder config, but this'll keep us from doing unnecessary per-frame calculations on pipeline state setup
		if (pass_cache.pipeline_states.find(pass->pass_config.name) != pass_cache.pipeline_states.end())
		{
			pass->pipeline_state_id = pass_cache.pipeline_states[pass->pass_config.name];
			CACHE_FETCH(PipelineState, pass->pipeline_state_id)->bind(gfx_context, command_buffer);
			return;
		}

		if (pass_cache.descriptors.find(pass->pass_config.name) == pass_cache.descriptors.end())
		{
			pass_cache.descriptors[pass->pass_config.name] = DescriptorDataList();
		}

		std::vector<DescriptorLayoutID> descriptor_layouts;

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

				state_builder.derive_shader_layout(descriptor_layouts);

				pass->pipeline_state_id = state_builder.finish();
			}
			else
			{
				PipelineGraphicsStateBuilder state_builder = PipelineGraphicsStateBuilder::create_default(gfx_context->get_window())
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

				state_builder.derive_shader_layout(descriptor_layouts);

				pass->pipeline_state_id = state_builder.finish();
			}
		}

		if (pass->pipeline_state_id != 0)
		{
			CACHE_FETCH(PipelineState, pass->pipeline_state_id)->bind(gfx_context, command_buffer);
		}

		pass_cache.descriptors[pass->pass_config.name].descriptor_layouts = descriptor_layouts;
		pass_cache.descriptors[pass->pass_config.name].descriptor_sets.resize(descriptor_layouts.size(), nullptr);
	}

	void RenderGraph::setup_pass_descriptors(class GraphicsContext* const gfx_context, RGPass* pass, void* command_buffer)
	{
		DescriptorDataList& pass_descriptor_data = pass_cache.descriptors[pass->pass_config.name];

		if (!pass_descriptor_data.descriptor_layouts.empty())
		{
			// Update global descriptor set
			{
				const uint32_t global_type_index = static_cast<uint32_t>(DescriptorSetType::Global);

				DescriptorSet*& global_descriptor_set = pass_descriptor_data.descriptor_sets[global_type_index];
				DescriptorLayoutID& global_descriptor_layout = pass_descriptor_data.descriptor_layouts[global_type_index];

				if (global_descriptor_set == nullptr)
				{
					global_descriptor_set = DescriptorHelpers::new_descriptor_set_with_layout(gfx_context, global_descriptor_layout);

					const uint32_t current_buffered_frame = gfx_context->get_buffered_frame_number();

					DescriptorLayout* const layout = CACHE_FETCH(DescriptorLayout, global_descriptor_layout);
					std::vector<DescriptorBinding>& descriptor_bindings = layout->get_bindings();

					std::vector<DescriptorWrite> descriptor_writes;
					for (uint32_t j = 0; j < descriptor_bindings.size(); ++j)
					{
						assert(j < queued_buffer_global_writes[current_buffered_frame].size());
						DescriptorWrite& new_write = descriptor_writes.emplace_back();
						new_write.slot = descriptor_bindings[j].slot;
						new_write.type = descriptor_bindings[j].type;
						new_write.count = descriptor_bindings[j].count;
						new_write.set = global_descriptor_set;
						new_write.buffer_desc = queued_buffer_global_writes[current_buffered_frame][j];
					}

					DescriptorHelpers::write_descriptors(gfx_context, global_descriptor_set, descriptor_writes);
				}
			}
		
			// Update per-pass descriptor set and create it if necessary
			{
				const uint32_t pass_type_index = static_cast<uint32_t>(DescriptorSetType::Pass);

				DescriptorSet*& pass_descriptor_set = pass_descriptor_data.descriptor_sets[pass_type_index];
				DescriptorLayoutID pass_descriptor_layout = pass_descriptor_data.descriptor_layouts[pass_type_index];

				DescriptorLayout* const layout = CACHE_FETCH(DescriptorLayout, pass_descriptor_layout);
				std::vector<DescriptorBinding>& descriptor_bindings = layout->get_bindings();

				const auto write_descriptors = [&](bool b_persistent_resources)
				{
					std::vector<DescriptorWrite> descriptor_writes;
					for (uint32_t j = 0; j < descriptor_bindings.size(); ++j)
					{
						if (current_registry->resource_metadata[pass->parameters.inputs[j]].b_is_persistent == b_persistent_resources)
						{
							DescriptorWrite& new_write = descriptor_writes.emplace_back();
							new_write.slot = descriptor_bindings[j].slot;
							new_write.type = descriptor_bindings[j].type;
							new_write.count = descriptor_bindings[j].count;
							new_write.set = pass_descriptor_set;

							if (new_write.type == DescriptorType::Image)
							{
								Image* const image = CACHE_FETCH(Image, current_registry->resource_metadata[pass->parameters.inputs[j]].physical_id);
								new_write.buffer_desc.buffer = image;
							}
							else
							{
								Buffer* const buffer = CACHE_FETCH(Buffer, current_registry->resource_metadata[pass->parameters.inputs[j]].physical_id);
								new_write.buffer_desc.buffer = buffer->get();
								new_write.buffer_desc.buffer_range = buffer->get_size();
								new_write.buffer_desc.buffer_size = buffer->get_size();
							}
						}
					}
					DescriptorHelpers::write_descriptors(gfx_context, pass_descriptor_set, descriptor_writes);
				};

				if (pass_descriptor_set == nullptr)
				{
					pass_descriptor_set = DescriptorHelpers::new_descriptor_set_with_layout(gfx_context, pass_descriptor_layout);
					const bool b_write_only_persistent_resources = true;
					write_descriptors(b_write_only_persistent_resources);
				}

				{
					const bool b_write_only_persistent_resources = false;
					write_descriptors(b_write_only_persistent_resources);
				}
			}
		}

		if (pass->pipeline_state_id != 0)
		{
			pass_descriptor_data.pipeline_layout = CACHE_FETCH(PipelineState, pass->pipeline_state_id)->get_state_data().layout;
		}
	}

	void RenderGraph::bind_pass_descriptors(class GraphicsContext* const gfx_context, RGPass* pass, void* command_buffer)
	{
		const bool b_is_compute_pass = (pass->pass_config.flags & RenderPassFlags::Compute) != RenderPassFlags::None;

		DescriptorDataList& pass_descriptor_data = pass_cache.descriptors[pass->pass_config.name];

		if (!pass_descriptor_data.descriptor_sets.empty())
		{
			// Bind global descriptors
			const uint32_t global_type_index = static_cast<uint32_t>(DescriptorSetType::Global);

			DescriptorSet*& global_descriptor_set = pass_descriptor_data.descriptor_sets[global_type_index];

			if (global_descriptor_set != nullptr)
			{
				global_descriptor_set->bind(
					gfx_context,
					command_buffer,
					pass_descriptor_data.pipeline_layout,
					b_is_compute_pass ? PipelineStateType::Compute : PipelineStateType::Graphics,
					global_type_index
				);
			}

			// Bind pass descriptors
			{
				const uint32_t pass_type_index = static_cast<uint32_t>(DescriptorSetType::Pass);

				DescriptorSet*& pass_descriptor_set = pass_descriptor_data.descriptor_sets[pass_type_index];

				if (pass_descriptor_set != nullptr)
				{
					pass_descriptor_set->bind(
						gfx_context,
						command_buffer,
						pass_descriptor_data.pipeline_layout,
						b_is_compute_pass ? PipelineStateType::Compute : PipelineStateType::Graphics,
						pass_type_index
					);
				}
			}
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
			RGResourceMetadata& resource_meta = current_registry->resource_metadata[input_resource];
			if (resource_meta.physical_id != 0 && resource_meta.last_user == pass->handle && !resource_meta.b_is_persistent)
			{
				// TODO: Aliasing
			}
		}
		for (RGResourceHandle output_resource : pass->parameters.outputs)
		{
			RGResourceMetadata& resource_meta = current_registry->resource_metadata[output_resource];
			if (resource_meta.physical_id != 0 && resource_meta.last_user == pass->handle && !resource_meta.b_is_persistent)
			{
				// TODO: Aliasing
			}
		}
	}

	void RenderGraph::free_physical_resources(class GraphicsContext* const gfx_context)
	{
		// TODO: Look into aliasing memory for expired resources as opposed to just flat out deleting them (though that should also be done)
		current_registry->resource_deletion_queue.flush();
	}
}
