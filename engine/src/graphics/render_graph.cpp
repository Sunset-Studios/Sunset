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

namespace Sunset
{
	void RenderGraph::initialize(GraphicsContext* const gfx_context)
	{
		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			pass_cache.global_descriptor_set[i] = nullptr;
		}
	}

	void RenderGraph::destroy(GraphicsContext* const gfx_context)
	{
		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			reset(gfx_context, i);
			registries[i].resource_deletion_queue.flush();
		}
	}

	void RenderGraph::begin(GraphicsContext* const gfx_context)
	{
		ZoneScopedN("RenderGraph::begin");
		reset(gfx_context, gfx_context->get_buffered_frame_number());
	}

	Sunset::RGResourceHandle RenderGraph::create_image(class GraphicsContext* const gfx_context, const AttachmentConfig& config, int32_t buffered_frame_number)
	{
		RGImageResource* const new_image_resource = image_resource_allocator[buffered_frame_number].get_new();
		new_image_resource->config = config;
		new_image_resource->config.flags |= ImageFlags::Transient;

		{
			RenderGraphRegistry& registry = registries[buffered_frame_number];
			const uint32_t index = registry.image_resources.size();
			registry.image_resources.push_back(new_image_resource);
			new_image_resource->handle = create_graph_resource_handle(index, static_cast<RGResourceType>(ResourceType::Image), 1);

			registry.all_resource_handles.push_back(new_image_resource->handle);
			registry.resource_metadata.insert({ new_image_resource->handle, {} });
			registry.resource_metadata[new_image_resource->handle].b_is_bindless = config.is_bindless;
		}

		return new_image_resource->handle;
	}

	Sunset::RGResourceHandle RenderGraph::register_image(class GraphicsContext* const gfx_context, ImageID image, int32_t buffered_frame_number)
	{
		Image* const image_obj = CACHE_FETCH(Image, image);
		RGImageResource* const new_image_resource = image_resource_allocator[buffered_frame_number].get_new();
		new_image_resource->config = image_obj->get_attachment_config();

		{
			RenderGraphRegistry& registry = registries[buffered_frame_number];
			const uint32_t index = registry.image_resources.size();
			registry.image_resources.push_back(new_image_resource);
			new_image_resource->handle = create_graph_resource_handle(index, static_cast<RGResourceType>(ResourceType::Image), 1);

			registry.all_resource_handles.push_back(new_image_resource->handle);
			registry.resource_metadata.insert({ new_image_resource->handle, {} });
			registry.resource_metadata[new_image_resource->handle].physical_id = image;
			registry.resource_metadata[new_image_resource->handle].b_is_persistent = true;
			registry.resource_metadata[new_image_resource->handle].b_is_bindless = new_image_resource->config.is_bindless;
		}

		return new_image_resource->handle;
	}

	Sunset::RGResourceHandle RenderGraph::create_buffer(class GraphicsContext* const gfx_context, const BufferConfig& config, int32_t buffered_frame_number)
	{
		RGBufferResource* const new_buffer_resource = buffer_resource_allocator[buffered_frame_number].get_new();
		new_buffer_resource->config = config;
		new_buffer_resource->config.type |= BufferType::Transient;

		{
			RenderGraphRegistry& registry = registries[buffered_frame_number];
			const uint32_t index = registry.buffer_resources.size();
			registry.buffer_resources.push_back(new_buffer_resource);
			new_buffer_resource->handle = create_graph_resource_handle(index, static_cast<RGResourceType>(ResourceType::Buffer), 1);

			registry.all_resource_handles.push_back(new_buffer_resource->handle);
			registry.resource_metadata.insert({ new_buffer_resource->handle, {} });
			registry.resource_metadata[new_buffer_resource->handle].b_is_bindless = config.b_is_bindless;
		}

		return new_buffer_resource->handle;
	}

	Sunset::RGResourceHandle RenderGraph::register_buffer(class GraphicsContext* const gfx_context, BufferID buffer, int32_t buffered_frame_number)
	{
		Buffer* const buffer_obj = CACHE_FETCH(Buffer, buffer);
		RGBufferResource* const new_buffer_resource = buffer_resource_allocator[buffered_frame_number].get_new();
		new_buffer_resource->config = buffer_obj->get_buffer_config();

		{
			RenderGraphRegistry& registry = registries[buffered_frame_number];
			const uint32_t index = registry.buffer_resources.size();
			registry.buffer_resources.push_back(new_buffer_resource);
			new_buffer_resource->handle = create_graph_resource_handle(index, static_cast<RGResourceType>(ResourceType::Buffer), 1);

			registry.all_resource_handles.push_back(new_buffer_resource->handle);
			registry.resource_metadata.insert({ new_buffer_resource->handle, {} });
			registry.resource_metadata[new_buffer_resource->handle].physical_id = buffer;
			registry.resource_metadata[new_buffer_resource->handle].b_is_persistent = true;
			registry.resource_metadata[new_buffer_resource->handle].b_is_bindless = new_buffer_resource->config.b_is_bindless;
		}

		return new_buffer_resource->handle;
	}

	Sunset::RGPassHandle RenderGraph::add_pass(class GraphicsContext* const gfx_context, Identity name, RenderPassFlags pass_type, int32_t buffered_frame_number, const RGPassParameters& params, std::function<void(RenderGraph&, RGFrameData&, void*)> execution_callback)
	{
		#if READABLE_STRINGS
		const std::string pass_name_str = name.string + std::to_string(buffered_frame_number);
		const Identity pass_name = pass_name_str.c_str();
		#else
		const Identity pass_name = name.computed_hash + buffered_frame_number;
		#endif

		RGPass* const pass = render_pass_allocator[buffered_frame_number].get_new();
		pass->pass_config = { .name = pass_name, .flags = pass_type };
		pass->parameters = params;
		pass->executor = execution_callback;

		uint32_t index{ 0 };
		{
			RenderGraphRegistry& registry = registries[buffered_frame_number];
			index = registry.render_passes.size();
			registry.render_passes.push_back(pass);
			nonculled_passes[buffered_frame_number].push_back(index);
			pass->handle = index;

			update_reference_counts(pass, registry);
			update_resource_param_producers_and_consumers(pass, registry);
			update_present_pass_status(pass, registry);
		}

		return index;
	}

	Sunset::RGPassHandle RenderGraph::add_pass(class GraphicsContext* const gfx_context, Identity name, RenderPassFlags pass_type, int32_t buffered_frame_number, std::function<void(RenderGraph&, RGFrameData&, void*)> execution_callback)
	{
		#if READABLE_STRINGS
		const std::string pass_name_str = name.string + std::to_string(buffered_frame_number);
		const Identity pass_name = pass_name_str.c_str();
		#else
		const Identity pass_name = name.computed_hash + buffered_frame_number;
		#endif

		RGPass* const pass = render_pass_allocator[buffered_frame_number].get_new();
		pass->pass_config = { .name = pass_name, .flags = pass_type };
		pass->executor = execution_callback;

		uint32_t index{ 0 };
		{
			RenderGraphRegistry& registry = registries[buffered_frame_number];
			index = registry.render_passes.size();
			registry.render_passes.push_back(pass);
			nonculled_passes[buffered_frame_number].push_back(index);
		}

		pass->handle = index;
		pass->reference_count = 1;

		return index;
	}

	void RenderGraph::add_pass_resource_barrier(RGResourceHandle resource, RGPassHandle pass, AccessFlags dst_access, ImageLayout dst_layout, int32_t buffered_frame_number)
	{
		RenderGraphRegistry& registry = registries[buffered_frame_number];

		if (auto it = registry.resource_metadata.find(resource); it != registry.resource_metadata.end())
		{
			RGResourceMetadata& metadata = (*it).second;

			const ResourceType resource_type = static_cast<ResourceType>(get_graph_resource_type(resource));

			if (RGPass* const pass_obj = registry.render_passes[pass])
			{
				const size_t num_passes = registry.render_passes.size();

				if (metadata.access_flags.size() < num_passes)
				{
					metadata.access_flags.resize(registry.render_passes.size(), AccessFlags::None);
				}
				if (metadata.layouts.size() < num_passes)
				{
					metadata.layouts.resize(registry.render_passes.size(), ImageLayout::Undefined);
				}

				metadata.access_flags[pass] = dst_access;
				metadata.layouts[pass] = dst_layout;
			}
		}
	}

	void RenderGraph::update_reference_counts(RGPass* pass, RenderGraphRegistry& registry)
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

	void RenderGraph::update_resource_param_producers_and_consumers(RGPass* pass, RenderGraphRegistry& registry)
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

	void RenderGraph::update_present_pass_status(RGPass* pass, RenderGraphRegistry& registry)
	{
		pass->pass_config.b_is_present_pass = (pass->pass_config.flags & RenderPassFlags::Present) != RenderPassFlags::None;
		// Do not cull present passes as these are usually last and have resources that have no future uses, so would get culled otherwise
		pass->parameters.b_force_keep_pass = pass->parameters.b_force_keep_pass || pass->pass_config.b_is_present_pass;
	}

	void RenderGraph::cull_graph_passes(class GraphicsContext* const gfx_context, int32_t buffered_frame_number)
	{
		std::unordered_set<RGPassHandle> passes_to_cull;
		std::vector<RGResourceHandle> unused_stack;

		RenderGraphRegistry& registry = registries[buffered_frame_number];

		// Lambda to cull a producer pass when it is no longer referenced and pop its inputs onto the unused stack
		auto update_producer_and_subresource_ref_counts = [this, &registry, &unused_stack, &passes_to_cull](const std::vector<RGPassHandle> producers)
		{
			for (RGPassHandle pass_handle : producers)
			{
				RGPass* const producer_pass = registry.render_passes[pass_handle];

				// Do not cull passes marked as "force keep" 
				if (producer_pass->parameters.b_force_keep_pass)
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
		for (int i = nonculled_passes[buffered_frame_number].size() - 1; i >= 0; --i)
		{
			RGPassHandle pass = nonculled_passes[buffered_frame_number][i];
			if (passes_to_cull.find(pass) != passes_to_cull.end())
			{
				nonculled_passes[buffered_frame_number].erase(nonculled_passes[buffered_frame_number].begin() + i);
			}
		}
	}

	void RenderGraph::compute_resource_first_and_last_users(class GraphicsContext* const gfx_context, int32_t buffered_frame_number)
	{
		RenderGraphRegistry& registry = registries[buffered_frame_number];

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

	void RenderGraph::compute_resource_barriers(class GraphicsContext* const gfx_context, int32_t buffered_frame_number)
	{
		RenderGraphRegistry& registry = registries[buffered_frame_number];

		const auto get_access_flags_for_resource_and_pass_type = [=](RGResourceHandle resource, ResourceType resource_type, RenderPassFlags pass_flags, bool b_input_resource)
		{
			const RGResourceIndex resource_index = get_graph_resource_index(resource);
			RGImageResource* const image_resource = registry.image_resources[resource_index];
			const bool b_is_depth_stencil = (image_resource->config.flags & ImageFlags::DepthStencil) != ImageFlags::None;
			const bool b_is_depth_stencil_load = b_is_depth_stencil && !image_resource->config.attachment_clear;

			if (!b_input_resource && !b_is_depth_stencil_load)
			{
				if (resource_type == ResourceType::Image && (pass_flags & RenderPassFlags::Graphics) != RenderPassFlags::None)
				{
					return b_is_depth_stencil ? AccessFlags::DepthStencilAttachmentWrite : AccessFlags::ColorAttachmentWrite;
				}
				else
				{
					return AccessFlags::ShaderWrite;
				}
			}
			return AccessFlags::ShaderRead;
		};

		const auto get_image_layout_for_resource_and_pass_type = [=](RGResourceHandle resource, ResourceType resource_type, RenderPassFlags pass_flags, bool b_input_resource)
		{
			const RGResourceIndex resource_index = get_graph_resource_index(resource);
			RGImageResource* const image_resource = registry.image_resources[resource_index];
			const bool b_is_depth_stencil = (image_resource->config.flags & ImageFlags::DepthStencil) != ImageFlags::None;
			const bool b_is_depth_stencil_load = b_is_depth_stencil && !image_resource->config.attachment_clear;

			if (!b_input_resource && !b_is_depth_stencil_load)
			{
				if (resource_type == ResourceType::Image && (pass_flags & RenderPassFlags::Graphics) != RenderPassFlags::None)
				{
					return b_is_depth_stencil ? ImageLayout::DepthStencilAttachment : ImageLayout::ColorAttachment;
				}
				else
				{
					return ImageLayout::General;
				}
			}
			return ImageLayout::ShaderReadOnly;
		};

		const size_t num_passes = registry.render_passes.size();
		// For each resource we compute the per-pass access masks and image layouts. We can then diff these
		// when executing passes based on prev -> current in order to determine whether the resource needs
		// a memory barrier.
		for (RGResourceHandle resource : registry.all_resource_handles)
		{
			if (auto it = registry.resource_metadata.find(resource); it != registry.resource_metadata.end())
			{
				RGResourceMetadata& metadata = (*it).second;
				if (metadata.reference_count == 0)
				{
					continue;
				}

				const ResourceType resource_type = static_cast<ResourceType>(get_graph_resource_type(resource));

				std::vector<RGPassHandle> all_users;
				all_users.insert(all_users.end(), metadata.producers.begin(), metadata.producers.end());
				all_users.insert(all_users.end(), metadata.consumers.begin(), metadata.consumers.end());
				std::sort(all_users.begin(), all_users.end());

				for (RGPassHandle pass : all_users)
				{
					if (pass < metadata.first_user || pass > metadata.last_user)
					{
						continue;
					}

					RGPass* const pass_obj = registry.render_passes[pass];

					if (metadata.access_flags.size() < num_passes)
					{
						metadata.access_flags.resize(registry.render_passes.size(), AccessFlags::None);
					}
					if (metadata.layouts.size() < num_passes)
					{
						metadata.layouts.resize(registry.render_passes.size(), ImageLayout::Undefined);
					}

					// Do not touch access/layout metadata that has already been set as this has most likely been done manually via a call to add_pass_resource_barrier
					if (metadata.access_flags[pass] == AccessFlags::None && metadata.layouts[pass] == ImageLayout::Undefined)
					{
						const bool b_is_input_for_pass = std::find(metadata.producers.begin(), metadata.producers.end(), pass) == metadata.producers.end();
						metadata.access_flags[pass] = get_access_flags_for_resource_and_pass_type(resource, resource_type, pass_obj->pass_config.flags, b_is_input_for_pass);
						metadata.layouts[pass] = get_image_layout_for_resource_and_pass_type(resource, resource_type, pass_obj->pass_config.flags, b_is_input_for_pass);
					}
				}
			}
		}
	}

	void RenderGraph::compile(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, int32_t buffered_frame_number)
	{
		cull_graph_passes(gfx_context, buffered_frame_number);
		compute_resource_first_and_last_users(gfx_context, buffered_frame_number);
		compute_resource_barriers(gfx_context, buffered_frame_number);
		// TODO: Not entirely sure how to do this yet (with VMA?) but walk resource lifetimes to account for how much GPU memory should be allocated up front for aliasing
	}

	void RenderGraph::submit(GraphicsContext* const gfx_context, class Swapchain* const swapchain, int32_t buffered_frame_number, bool b_offline)
	{
		RenderGraphRegistry& registry = registries[buffered_frame_number];

		compile(gfx_context, swapchain, buffered_frame_number);

		if (nonculled_passes[buffered_frame_number].empty())
		{
			return;
		}

		registry.barrier_batcher.begin(gfx_context, PipelineStageType::AllCommands);

		// TODO: switch the queue based on the pass type
		void* cmd_buffer = gfx_context->get_command_queue(DeviceQueueType::Graphics)->begin_one_time_buffer_record(gfx_context, buffered_frame_number);

		{
			RGFrameData frame_data
			{
				.gfx_context = gfx_context,
				.buffered_frame_number = buffered_frame_number,
				.resource_deletion_queue = &registry.resource_deletion_queue
			};

			for (RGPassHandle pass_handle : nonculled_passes[buffered_frame_number])
			{
				execute_pass(gfx_context, swapchain, registry.render_passes[pass_handle], frame_data, cmd_buffer);
			}
		}

		// TODO: switch the queue based on the pass type
		gfx_context->get_command_queue(DeviceQueueType::Graphics)->end_one_time_buffer_record(gfx_context, buffered_frame_number);
		// TODO: switch the queue based on the pass type
		gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit(gfx_context, buffered_frame_number, b_offline);
	}

	void RenderGraph::queue_global_descriptor_writes(class GraphicsContext* const gfx_context, uint32_t buffered_frame, const std::initializer_list<DescriptorBufferDesc>& buffers)
	{
		assert(buffered_frame >= 0 && buffered_frame < MAX_BUFFERED_FRAMES);
		queued_buffer_global_writes[buffered_frame] = buffers;
	}

	size_t RenderGraph::get_physical_resource(RGResourceHandle resource, int32_t buffered_frame_number)
	{
		RenderGraphRegistry& registry = registries[buffered_frame_number];

		if (registry.resource_metadata.find(resource) != registry.resource_metadata.end())
		{
			return registry.resource_metadata[resource].physical_id;
		}

		return 0;
	}

	void RenderGraph::reset(class GraphicsContext* const gfx_context, int32_t current_buffered_frame)
	{
		free_physical_resources(gfx_context, current_buffered_frame);

		nonculled_passes[current_buffered_frame].clear();

		{
			RenderGraphRegistry* const registry = &registries[current_buffered_frame];
			registry->all_resource_handles.clear();
			registry->buffer_resources.clear();
			registry->image_resources.clear();
			registry->all_bindless_resource_handles.clear();
			registry->render_passes.clear();
			registry->resource_metadata.clear();
			registry->b_global_set_bound = false;
		}

		image_resource_allocator[current_buffered_frame].reset();
		buffer_resource_allocator[current_buffered_frame].reset();
		render_pass_allocator[current_buffered_frame].reset();
	}

	void RenderGraph::execute_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGPass* pass, RGFrameData& frame_data, void* command_buffer)
	{
		assert(pass != nullptr);

		if ((pass->pass_config.flags & RenderPassFlags::GraphLocal) != RenderPassFlags::None)
		{
			frame_data.global_descriptor_set = pass_cache.global_descriptor_set[frame_data.buffered_frame_number];

			pass->executor(*this, frame_data, command_buffer);
		}
		else
		{
			setup_physical_pass_and_resources(gfx_context, swapchain, pass, frame_data, command_buffer);

			{
				const uint32_t pass_type_index = static_cast<uint32_t>(DescriptorSetType::Pass);

				DescriptorDataList& descriptor_data_list = pass_cache.descriptors[frame_data.buffered_frame_number][pass->pass_config.name];
				if (pass_type_index < descriptor_data_list.descriptor_sets.size())
				{
					frame_data.pass_descriptor_set = descriptor_data_list.descriptor_sets[pass_type_index];
				}
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

				physical_pass->begin_pass(gfx_context, pass->pass_config.b_is_present_pass ? swapchain->get_current_image_index(frame_data.buffered_frame_number) : 0, command_buffer);
				pass->executor(*this, frame_data, command_buffer);
				physical_pass->end_pass(gfx_context, command_buffer);
			}

			update_transient_resources(gfx_context, pass, frame_data.buffered_frame_number);
		}
	}

	void RenderGraph::setup_physical_pass_and_resources(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGPass* pass, RGFrameData& frame_data, void* command_buffer)
	{
		RenderGraphRegistry& registry = registries[frame_data.buffered_frame_number];

		const bool b_is_graphics_pass = (pass->pass_config.flags & RenderPassFlags::Graphics) != RenderPassFlags::None;

		const auto setup_resource = [this, gfx_context, swapchain, pass, &frame_data, &registry, b_is_graphics_pass](RGResourceHandle resource, uint32_t resource_params_index, bool b_is_input_resource)
		{
			if (auto it = registry.resource_metadata.find(resource); it != registry.resource_metadata.end())
			{
				setup_physical_resource(gfx_context, swapchain, pass, resource, frame_data, b_is_graphics_pass, b_is_input_resource);
				if (b_is_graphics_pass)
				{
					tie_resource_to_pass_config_attachments(gfx_context, resource, pass, frame_data.buffered_frame_number, resource_params_index, b_is_input_resource);
				}
				if (b_is_input_resource)
				{
					setup_pass_input_resource_bindless_type(gfx_context, resource, pass, frame_data.buffered_frame_number);
				}
			}	
		};

		// Setup resource used by the render pass. If they are output/input attachments we will handle those here as well but cache those off
		// as those necessarily need caching along with the render passes.
		for (uint32_t i = 0; i < pass->parameters.inputs.size(); ++i)
		{
			RGResourceHandle input_resource = pass->parameters.inputs[i];
			setup_resource(input_resource, i, true);
		}
		for (uint32_t i = 0; i < pass->parameters.outputs.size(); ++i)
		{
			RGResourceHandle output_resource = pass->parameters.outputs[i];
			setup_resource(output_resource, i, false);
		}

		// Create our physical render pass (is internally cached if necessary using a hash based on the pass config)
		pass->physical_id = RenderPassFactory::create_default(gfx_context, swapchain, pass->pass_config, true);

		// Setup render pass pipeline state if render pass-level shader stages were provided. Otherwise, pipeline state should be handled
		// and bound by render pass callbacks.
		setup_pass_pipeline_state(gfx_context, pass, frame_data, command_buffer);

		setup_pass_descriptors(gfx_context, pass, frame_data, command_buffer);

		bind_pass_descriptors(gfx_context, pass, frame_data, command_buffer);

		push_pass_constants(gfx_context, pass, command_buffer);

		// Terribly weak synchronization here, but render graph barriers are meant to coarsely cover synch cases since we can't know exactly what access and layout states
		// resources will be in, and therefore we have no guarantees we can use to guard against using incompatible access masks for a given stage. More specific synch should
		// be handled by the application.
		PipelineStageType barrier_execution_stage = (pass->pass_config.flags & RenderPassFlags::Compute) != RenderPassFlags::None
			? PipelineStageType::AllCommands
			: PipelineStageType::AllGraphics;
		registry.barrier_batcher.execute(gfx_context, command_buffer, barrier_execution_stage, pass->pass_config.name);
	}

	void RenderGraph::setup_physical_resource(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGPass* pass, RGResourceHandle resource, RGFrameData& frame_data, bool b_is_graphics_pass, bool b_is_input_resource)
	{
		RenderGraphRegistry& registry = registries[frame_data.buffered_frame_number];

		const ResourceType resource_type = static_cast<ResourceType>(get_graph_resource_type(resource));
		const RGResourceIndex resource_index = get_graph_resource_index(resource);
		if (resource_type == ResourceType::Buffer)
		{
			RGBufferResource* const buffer_resource = registry.buffer_resources[resource_index];
			// We check for an empty ID first because registered external resources would have already had this field populated
			if (registry.resource_metadata[resource].physical_id == 0)
			{
				buffer_resource->config.name.computed_hash += frame_data.buffered_frame_number;

				registry.resource_metadata[resource].physical_id = BufferFactory::create(gfx_context, buffer_resource->config, false);

				if (!registry.resource_metadata[resource].b_is_persistent)
				{
					registry.resource_deletion_queue.remove_execution(registry.resource_metadata[resource].physical_id);
					registry.resource_deletion_queue.push_execution([gfx_context, physical_id = registry.resource_metadata[resource].physical_id]()
						{
							CACHE_DELETE(Buffer, physical_id, gfx_context);
						},
						registry.resource_metadata[resource].physical_id,
						registry.resource_metadata[resource].max_frame_lifetime
					);
				}
			}

			// Add a buffer barrier if the requested access flags seem different than the current buffer state
			if (!registry.resource_metadata[resource].access_flags.empty())
			{
				Buffer* const buffer = CACHE_FETCH(Buffer, registry.resource_metadata[resource].physical_id);
				const AccessFlags access_flags = registry.resource_metadata[resource].access_flags[pass->handle];
				if (buffer->get_access_flags() != access_flags)
				{
					registry.barrier_batcher.add_buffer_barrier(
						gfx_context,
						buffer,
						pass->pass_config.name,
						access_flags
					);
				}
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
				registry.resource_metadata[resource].b_is_persistent |= b_is_persistent;

				image_resource->config.name.computed_hash += frame_data.buffered_frame_number;

				registry.resource_metadata[resource].physical_id = ImageFactory::create(gfx_context, image_resource->config, b_is_persistent);

				if (!registry.resource_metadata[resource].b_is_persistent)
				{
					registry.resource_deletion_queue.remove_execution(registry.resource_metadata[resource].physical_id);
					registry.resource_deletion_queue.push_execution([gfx_context, physical_id = registry.resource_metadata[resource].physical_id]()
						{
							CACHE_DELETE(Image, physical_id, gfx_context);
						},
						registry.resource_metadata[resource].physical_id,
						registry.resource_metadata[resource].max_frame_lifetime
					);
				}
			}

			// Add an image barrier if the requested access flags and image layout seem different than the current image state
			if (!registry.resource_metadata[resource].access_flags.empty() && !registry.resource_metadata[resource].layouts.empty())
			{
				Image* const image = CACHE_FETCH(Image, registry.resource_metadata[resource].physical_id);
				const AccessFlags access_flags = registry.resource_metadata[resource].access_flags[pass->handle];
				const ImageLayout layout = registry.resource_metadata[resource].layouts[pass->handle];
				if (image->get_access_flags() != access_flags || image->get_layout() != layout)
				{
					registry.barrier_batcher.add_image_barrier(
						gfx_context,
						image,
						pass->pass_config.name,
						access_flags,
						layout
					);
				}
			}
		}
	}

	void RenderGraph::tie_resource_to_pass_config_attachments(class GraphicsContext* const gfx_context, RGResourceHandle resource, RGPass* pass, int32_t buffered_frame_number, uint32_t resource_params_index, bool b_is_input_resource)
	{
		RenderGraphRegistry& registry = registries[buffered_frame_number];

		const ResourceType resource_type = static_cast<ResourceType>(get_graph_resource_type(resource));
		const RGResourceIndex resource_index = get_graph_resource_index(resource);
		if (resource_type == ResourceType::Image)
		{
			RGImageResource* const image_resource = registry.image_resources[resource_index];
			const bool b_is_local_load = (image_resource->config.flags & ImageFlags::LocalLoad) != ImageFlags::None;
			if (!b_is_input_resource || b_is_local_load)
			{
				const uint32_t image_view_index = pass->parameters.output_views.size() > resource_params_index ? pass->parameters.output_views[resource_params_index] : 0;
				// NOTE: We can set b_image_view_considers_layer_split here if we want to process an image view with multiple layers as a single layer of that image view
				pass->pass_config.attachments.push_back({ .image = registry.resource_metadata[resource].physical_id, .image_view_index = image_view_index/*, .b_image_view_considers_layer_split = false*/});
			}
		}
	}

	void RenderGraph::setup_pass_input_resource_bindless_type(class GraphicsContext* const gfx_context, RGResourceHandle resource, RGPass* pass, int32_t buffered_frame_number)
	{
		RenderGraphRegistry& registry = registries[buffered_frame_number];

		const ResourceType resource_type = static_cast<ResourceType>(get_graph_resource_type(resource));
		const RGResourceIndex resource_index = get_graph_resource_index(resource);
		if (resource_type == ResourceType::Image)
		{
			RGImageResource* const image_resource = registry.image_resources[resource_index];
			if (image_resource->config.is_bindless)
			{
				pass->parameters.bindless_inputs.push_back(resource);
			}
			else
			{
				pass->parameters.pass_inputs.push_back(resource);
			}
		}
		else if (resource_type == ResourceType::Buffer)
		{
			RGBufferResource* const buffer_resource = registry.buffer_resources[resource_index];
			if (buffer_resource->config.b_is_bindless)
			{
				pass->parameters.bindless_inputs.push_back(resource);
			}
			else
			{
				pass->parameters.pass_inputs.push_back(resource);
			}
		}
	}

	void RenderGraph::setup_pass_pipeline_state(class GraphicsContext* const gfx_context, RGPass* pass, RGFrameData& frame_data, void* command_buffer)
	{
		// We don't "need" the additional pipeline state caching here since internally pipeline states are already cached
		// based on the builder config, but this'll keep us from doing unnecessary per-frame calculations on pipeline state setup
		if (pass_cache.pipeline_states[frame_data.buffered_frame_number].find(pass->pass_config.name) != pass_cache.pipeline_states[frame_data.buffered_frame_number].end())
		{
			pass->pipeline_state_id = pass_cache.pipeline_states[frame_data.buffered_frame_number][pass->pass_config.name];
			CACHE_FETCH(PipelineState, pass->pipeline_state_id)->bind(gfx_context, command_buffer);
			return;
		}

		if (pass_cache.descriptors[frame_data.buffered_frame_number].find(pass->pass_config.name) == pass_cache.descriptors[frame_data.buffered_frame_number].end())
		{
			pass_cache.descriptors[frame_data.buffered_frame_number][pass->pass_config.name] = DescriptorDataList();
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

				if (shader_setup.push_constant_data.has_value())
				{
					state_builder.set_push_constants(shader_setup.push_constant_data.value());
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
				PipelineGraphicsStateBuilder state_builder = PipelineGraphicsStateBuilder::create_default(gfx_context->get_surface_resolution())
					.clear_shader_stages()
					.set_pass(pass->physical_id)
					.value();

				if (shader_setup.rasterizer_state.has_value())
				{
					state_builder.set_rasterizer_state(shader_setup.rasterizer_state.value());
				}

				if (shader_setup.attachment_blend.has_value())
				{
					state_builder.set_attachment_blend_state(shader_setup.attachment_blend.value());
				}

				if (shader_setup.primitive_topology_type.has_value())
				{
					state_builder.set_primitive_topology_type(shader_setup.primitive_topology_type.value());
				}

				const bool b_depth_test_enabled = shader_setup.b_depth_write_enabled.has_value() ? shader_setup.b_depth_write_enabled.value() : true;
				const CompareOperation depth_stencil_compare_op = shader_setup.depth_stencil_compare_op.has_value() ? shader_setup.depth_stencil_compare_op.value() : CompareOperation::LessOrEqual;
				state_builder.set_depth_stencil_state(true, b_depth_test_enabled, depth_stencil_compare_op);

				if (shader_setup.push_constant_data.has_value())
				{
					state_builder.set_push_constants(shader_setup.push_constant_data.value());
				}

				if (shader_setup.viewport.has_value())
				{
					const Viewport& viewport = shader_setup.viewport.value();
					state_builder.clear_viewports();
					state_builder.clear_scissors();
					state_builder.add_viewport(viewport);
					state_builder.add_scissor(viewport.x, viewport.y, viewport.width, viewport.height);
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
			pass_cache.pipeline_states[frame_data.buffered_frame_number][pass->pass_config.name] = pass->pipeline_state_id;
		}

		pass_cache.descriptors[frame_data.buffered_frame_number][pass->pass_config.name].descriptor_layouts = descriptor_layouts;
		pass_cache.descriptors[frame_data.buffered_frame_number][pass->pass_config.name].descriptor_sets.resize(descriptor_layouts.size(), nullptr);
	}

	void RenderGraph::setup_pass_descriptors(class GraphicsContext* const gfx_context, RGPass* pass, RGFrameData& frame_data, void* command_buffer)
	{
		RenderGraphRegistry& registry = registries[frame_data.buffered_frame_number];

		DescriptorDataList& pass_descriptor_data = pass_cache.descriptors[frame_data.buffered_frame_number][pass->pass_config.name];

		if (!pass_descriptor_data.descriptor_layouts.empty())
		{
			// Update global descriptor set (should happen once, lazily, on startup)
			if (pass_cache.global_descriptor_set[frame_data.buffered_frame_number] == nullptr)
			{
				const uint32_t global_type_index = static_cast<uint32_t>(DescriptorSetType::Global);

				const DescriptorLayoutID global_descriptor_layout = pass_descriptor_data.descriptor_layouts[global_type_index];

				pass_cache.global_descriptor_set[frame_data.buffered_frame_number] = DescriptorHelpers::new_descriptor_set_with_layout(gfx_context, global_descriptor_layout);

				DescriptorLayout* const layout = CACHE_FETCH(DescriptorLayout, global_descriptor_layout);
				std::vector<DescriptorBinding>& descriptor_bindings = layout->get_bindings();

				std::vector<DescriptorWrite> descriptor_writes;
				for (uint32_t j = 0; j < descriptor_bindings.size(); ++j)
				{
					assert(j < queued_buffer_global_writes[frame_data.buffered_frame_number].size());
					DescriptorWrite& new_write = descriptor_writes.emplace_back();
					new_write.slot = descriptor_bindings[j].slot;
					new_write.type = descriptor_bindings[j].type;
					new_write.count = descriptor_bindings[j].count;
					new_write.buffer_desc = queued_buffer_global_writes[frame_data.buffered_frame_number][j];
				}

				DescriptorHelpers::write_descriptors(gfx_context, pass_cache.global_descriptor_set[frame_data.buffered_frame_number], descriptor_writes);
			}

			// Write bindless pass resources
			if (!pass->parameters.b_skip_auto_descriptor_setup)
			{
				// Writes the bindless resources into the corresponding global binding tables. The resulting binding indices get passed as a struct as
				// part of the frame data for this pass, to allow custom handling by internal pass lambdas
				const size_t num_bindless_inputs = pass->parameters.bindless_inputs.size();

				frame_data.pass_bindless_resources.handles.clear();

				std::vector<DescriptorBindlessWrite> bindless_writes;
				bindless_writes.reserve(num_bindless_inputs);

				for (uint32_t j = 0; j < num_bindless_inputs; ++j)
				{
					const RGResourceHandle resource = pass->parameters.bindless_inputs[j];
					const ResourceType resource_type = static_cast<ResourceType>(get_graph_resource_type(resource));
					if (resource_type == ResourceType::Image)
					{
						std::vector<DescriptorBindlessWrite> image_writes = DescriptorHelpers::new_descriptor_image_bindless_writes(pass_cache.global_descriptor_set[frame_data.buffered_frame_number], registry.resource_metadata[resource].physical_id, pass->parameters.b_split_input_image_mips);
						bindless_writes.insert(
							bindless_writes.end(),
							image_writes.begin(),
							image_writes.end()
						);
					}
				}

				// Guarantees that storage image handles come after regular image handles
				std::stable_partition(bindless_writes.begin(), bindless_writes.end(),
					[](const DescriptorBindlessWrite& w) -> bool { return w.type == DescriptorType::Image; }
				);

				frame_data.pass_bindless_resources.handles.resize(bindless_writes.size(), -1);

				DescriptorHelpers::write_bindless_descriptors(gfx_context, bindless_writes, frame_data.pass_bindless_resources.handles.data());

				registry.all_bindless_resource_handles.insert(
					registry.all_bindless_resource_handles.end(),
					frame_data.pass_bindless_resources.handles.begin(),
					frame_data.pass_bindless_resources.handles.end()
				);

				frame_data.global_descriptor_set = pass_cache.global_descriptor_set[frame_data.buffered_frame_number];
			}

			{
				// Update per-pass descriptor set and write regular descriptor resources
				const uint32_t pass_type_index = static_cast<uint32_t>(DescriptorSetType::Pass);

				if (pass_type_index < pass_descriptor_data.descriptor_layouts.size())
				{
					DescriptorSet*& pass_descriptor_set = pass_descriptor_data.descriptor_sets[pass_type_index];
					DescriptorLayoutID pass_descriptor_layout = pass_descriptor_data.descriptor_layouts[pass_type_index];

					DescriptorLayout* const layout = CACHE_FETCH(DescriptorLayout, pass_descriptor_layout);
					std::vector<DescriptorBinding>& descriptor_bindings = layout->get_bindings();

					const auto write_descriptors = [&](bool b_persistent_resources)
					{
						// Writes regular per-pass descriptor bindings based on passed in inputs that are NOT bindless
						if (!pass->parameters.b_skip_auto_descriptor_setup)
						{
							std::vector<DescriptorWrite> descriptor_writes;
							for (uint32_t j = 0; j < descriptor_bindings.size(); ++j)
							{
								const RGResourceHandle corresponding_binding_resource = pass->parameters.pass_inputs[j];
								if (registry.resource_metadata[corresponding_binding_resource].b_is_persistent == b_persistent_resources)
								{
									DescriptorWrite& new_write = descriptor_writes.emplace_back();
									new_write.slot = descriptor_bindings[j].slot;
									new_write.type = descriptor_bindings[j].type;
									new_write.count = descriptor_bindings[j].count;

									if (new_write.type == DescriptorType::Image || new_write.type == DescriptorType::StorageImage)
									{
										Image* const image = CACHE_FETCH(Image, registry.resource_metadata[corresponding_binding_resource].physical_id);
										new_write.buffer_desc.buffer = image;
									}
									else
									{
										Buffer* const buffer = CACHE_FETCH(Buffer, registry.resource_metadata[corresponding_binding_resource].physical_id);
										new_write.buffer_desc.buffer = buffer->get();
										new_write.buffer_desc.buffer_range = buffer->get_size();
										new_write.buffer_desc.buffer_size = buffer->get_size();
									}
								}
							}
							DescriptorHelpers::write_descriptors(gfx_context, pass_descriptor_set, descriptor_writes);
						}
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

					frame_data.pass_descriptor_set = pass_descriptor_set;
				}
			}
		}

		if (pass->pipeline_state_id != 0)
		{
			pass_descriptor_data.pipeline_layout = CACHE_FETCH(PipelineState, pass->pipeline_state_id)->get_state_data().layout;
		}
	}

	void RenderGraph::bind_pass_descriptors(class GraphicsContext* const gfx_context, RGPass* pass, RGFrameData& frame_data, void* command_buffer)
	{
		RenderGraphRegistry& registry = registries[frame_data.buffered_frame_number];

		const bool b_is_compute_pass = (pass->pass_config.flags & RenderPassFlags::Compute) != RenderPassFlags::None;

		DescriptorDataList& pass_descriptor_data = pass_cache.descriptors[frame_data.buffered_frame_number][pass->pass_config.name];

		if (!pass_descriptor_data.descriptor_layouts.empty())
		{
			// Bind global descriptors
			const uint32_t global_type_index = static_cast<uint32_t>(DescriptorSetType::Global);

			// TODO: Can't currently guarantee that two passes will equate in their push constant ranges, which is
			// required for vulkan pipeline compatibility when trying to share a descriptor set across layouts,
			// so we bind the global descriptor set per-pass for now
			//if (pass_cache.global_descriptor_set[frame_data.buffered_frame_number] != nullptr && !registries[frame_data.buffered_frame_number].b_global_set_bound)
			{
				pass_cache.global_descriptor_set[frame_data.buffered_frame_number]->bind(
					gfx_context,
					command_buffer,
					pass_descriptor_data.pipeline_layout,
					b_is_compute_pass ? PipelineStateType::Compute : PipelineStateType::Graphics,
					global_type_index
				);
				registry.b_global_set_bound = true;
			}

			// Bind pass descriptors
			const uint32_t pass_type_index = static_cast<uint32_t>(DescriptorSetType::Pass);

			if (pass_type_index < pass_descriptor_data.descriptor_layouts.size())
			{
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
		if (pass->parameters.shader_setup.push_constant_data.has_value() && pass->parameters.shader_setup.push_constant_data->is_valid())
		{
			gfx_context->push_constants(command_buffer, pass->pipeline_state_id, pass->parameters.shader_setup.push_constant_data.value());
		}
	}

	void RenderGraph::update_transient_resources(class GraphicsContext* const gfx_context, RGPass* pass, int32_t buffered_frame_number)
	{
		RenderGraphRegistry& registry = registries[buffered_frame_number];

		// TODO: Look into aliasing memory for expired resources as opposed to just flat out deleting them (though that should also be done)
		for (RGResourceHandle input_resource : pass->parameters.inputs)
		{
			RGResourceMetadata& resource_meta = registry.resource_metadata[input_resource];
			if (resource_meta.physical_id != 0 && resource_meta.last_user == pass->handle && !resource_meta.b_is_persistent)
			{
				// TODO: Transient resource memory aliasing
			}
		}
		for (RGResourceHandle output_resource : pass->parameters.outputs)
		{
			RGResourceMetadata& resource_meta = registry.resource_metadata[output_resource];
			if (resource_meta.physical_id != 0 && resource_meta.last_user == pass->handle && !resource_meta.b_is_persistent)
			{
				// TODO: Transient resource memory aliasing
			}
		}
	}

	void RenderGraph::free_physical_resources(class GraphicsContext* const gfx_context, int32_t current_buffered_frame)
	{
		ZoneScopedN("RenderGraph::free_physical_resources");

		// TODO: Look into aliasing memory for expired resources as opposed to just flat out deleting them (though that should also be done)
		registries[current_buffered_frame].resource_deletion_queue.update();
		DescriptorHelpers::free_bindless_image_descriptors(gfx_context, pass_cache.global_descriptor_set[current_buffered_frame], registries[current_buffered_frame].all_bindless_resource_handles);
	}
}
