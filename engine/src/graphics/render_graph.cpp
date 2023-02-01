#include <graphics/render_graph.h>
#include <graphics/render_pass.h>
#include <graphics/graphics_context.h>
#include <graphics/command_queue.h>
#include <graphics/resource/swapchain.h>

#include <unordered_set>

namespace Sunset
{
	void RenderGraph::initialize(GraphicsContext* const gfx_context)
	{
	}

	void RenderGraph::destroy(GraphicsContext* const gfx_context)
	{
		for (RGPass* const pass : registry.render_passes)
		{
			if (pass != nullptr)
			{
				RenderPass* const physical_pass = CACHE_FETCH(RenderPass, pass->physical_id);
				physical_pass->destroy(gfx_context);
			}
		}
	}

	Sunset::RGResourceHandle RenderGraph::create_image(class GraphicsContext* const gfx_context, const AttachmentConfig& config)
	{
		RGImageResource* const new_image_resource = image_resource_allocator.get_new();
		new_image_resource->config = config;

		const uint32_t index = registry.image_resources.size();
		registry.image_resources.push_back(new_image_resource);
		new_image_resource->handle = create_graph_resource_handle(index, static_cast<RGResourceType>(ResourceType::Image), 1);

		registry.all_resource_handles.push_back(new_image_resource->handle);
		registry.resource_metadata.insert(new_image_resource->handle, {});

		return new_image_resource->handle;
	}

	Sunset::RGResourceHandle RenderGraph::create_buffer(class GraphicsContext* const gfx_context, const BufferConfig& config)
	{
		RGBufferResource* const new_buffer_resource = buffer_resource_allocator.get_new();
		new_buffer_resource->config = config;

		const uint32_t index = registry.buffer_resources.size();
		registry.buffer_resources.push_back(new_buffer_resource);
		new_buffer_resource->handle = create_graph_resource_handle(index, static_cast<RGResourceType>(ResourceType::Buffer), 1);

		registry.all_resource_handles.push_back(new_buffer_resource->handle);
		registry.resource_metadata.insert(new_buffer_resource->handle, {});

		return new_buffer_resource->handle;
	}

	Sunset::RGPassHandle RenderGraph::add_pass(class GraphicsContext* const gfx_context, Identity name, RenderPassFlags pass_type, const RGPassParameters& params, std::function<void(RenderGraph&, void*)> execution_callback)
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

	void RenderGraph::cull_graph_passes(class GraphicsContext* const gfx_context)
	{
		std::unordered_set<RGPassHandle> passes_to_cull;
		std::vector<RGResourceHandle> unused_stack;

		// Lambda to cull a producer pass when it is no longer referenced and pop its inputs onto the stack
		auto update_producer_and_subresource_ref_counts = [this, &unused_stack, &passes_to_cull](const std::vector<RGPassHandle> producers)
		{
			for (RGPassHandle pass_handle : producers)
			{
				RGPass* const producer_pass = registry.render_passes[pass_handle];

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
	}

	void RenderGraph::submit(GraphicsContext* const gfx_context, class Swapchain* const swapchain)
	{
		compile(gfx_context, swapchain);

		// TODO: switch the queue based on the pass type
		void* buffer = gfx_context->get_command_queue(DeviceQueueType::Graphics)->begin_one_time_buffer_record(gfx_context);

		for (RGPassHandle pass_handle : nonculled_passes)
		{
			create_physical_pass_if_necessary(gfx_context, swapchain, pass_handle);
			execute_pass(gfx_context, swapchain, registry.render_passes[pass_handle], buffer);
			// TODO: Update transient resources
		}

		// TODO: switch the queue based on the pass type
		gfx_context->get_command_queue(DeviceQueueType::Graphics)->end_one_time_buffer_record(gfx_context);
		// TODO: switch the queue based on the pass type
		gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit(gfx_context);
	}

	void RenderGraph::reset()
	{
	}

	void RenderGraph::create_physical_pass_if_necessary(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGPassHandle pass)
	{
		RGPass* const graph_pass = registry.render_passes[pass];

		for (RGPassHandle input_resource : graph_pass->parameters.inputs)
		{
			create_physical_resource_if_necessary(gfx_context, swapchain, input_resource);
			// TODO: Need to bind inputs to descriptor sets and also push into pass config attachments if resource is an image
		}
		for (RGPassHandle output_resource : graph_pass->parameters.outputs)
		{
			create_physical_resource_if_necessary(gfx_context, swapchain, output_resource);
			// TODO: Need to push into pass config attachments if resource is an image
		}

		if (!registry.pass_cache.contains(graph_pass->pass_config.name))
		{
			if ((graph_pass->pass_config.flags | RenderPassFlags::Compute) != RenderPassFlags::None)
			{
				graph_pass->physical_id = RenderPassFactory::create_default_compute(gfx_context, graph_pass->pass_config);
			}
			else if ((graph_pass->pass_config.flags | RenderPassFlags::Main) != RenderPassFlags::None)
			{
				graph_pass->physical_id = RenderPassFactory::create_default_graphics(gfx_context, swapchain, graph_pass->pass_config);
			}
			registry.pass_cache.insert(graph_pass->pass_config.name);
		}
	}

	void RenderGraph::create_physical_resource_if_necessary(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGResourceHandle resource)
	{
		ResourceType resource_type = static_cast<ResourceType>(get_graph_resource_type(resource));
		if (resource_type == ResourceType::Buffer)
		{
			
		}
		else if (resource_type == ResourceType::Image)
		{

		}
	}

	void RenderGraph::execute_pass(class GraphicsContext* const gfx_context, class Swapchain* const swapchain, RGPass* pass, void* command_buffer)
	{
		assert(pass != nullptr && pass->physical_id != 0);
		RenderPass* const physical_pass = CACHE_FETCH(RenderPass, pass->physical_id);
		physical_pass->begin_pass(gfx_context, pass->pass_config.b_is_present_pass ? swapchain->get_current_image_index() : 0, command_buffer);
		pass->executor(*this, command_buffer);
		physical_pass->end_pass(gfx_context, command_buffer);
	}
}
