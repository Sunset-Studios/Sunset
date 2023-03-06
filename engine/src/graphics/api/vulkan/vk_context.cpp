#include <graphics/api/vulkan/vk_context.h>
#include <graphics/command_queue.h>
#include <graphics/resource/buffer.h>
#include <graphics/pipeline_state.h>
#include <graphics/descriptor.h>
#include <graphics/resource/image.h>
#include <core/common.h>
#include <window/window.h>

#include <SDL_vulkan.h>
#include <spirv_reflect.h>

namespace Sunset
{
	void VulkanContext::initialize(Window* const window)
	{
		vkb::InstanceBuilder builder;

		vkb::Instance instance_result = builder.set_app_name(ENGINE_NAME)
			.request_validation_layers(true)
			.require_api_version(1, 1, 0)
			.use_default_debug_messenger()
			.build()
			.value();

		state.instance = instance_result.instance;
		state.debug_messenger = instance_result.debug_messenger;

		create_surface(this, window);

		vkb::PhysicalDeviceSelector device_selector{ instance_result };
		state.physical_device = device_selector
			.set_minimum_version(1, 1)
			.set_surface(state.surface)
			.add_required_extension("VK_EXT_descriptor_indexing")
			.select()
			.value();

		vkb::DeviceBuilder device_builder{ state.physical_device };

		VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features = {};
		shader_draw_parameters_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
		shader_draw_parameters_features.pNext = nullptr;

		VkPhysicalDeviceDescriptorIndexingFeatures indexing_features = {};
		indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
		indexing_features.pNext = &shader_draw_parameters_features;

		VkPhysicalDeviceFeatures2 device_features = {};
		device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		device_features.pNext = &indexing_features;

		vkGetPhysicalDeviceFeatures2(state.physical_device.physical_device, &device_features);

		state.device = device_builder
			.add_pNext(&device_features)
			.build()
			.value();

		state.window = window;
		state.supports_bindless = indexing_features.descriptorBindingPartiallyBound && indexing_features.runtimeDescriptorArray;

		for (int16_t frame_number = 0; frame_number < MAX_BUFFERED_FRAMES; ++frame_number)
		{
			state.frame_sync_primitives[frame_number].render_semaphore = state.sync_pool.new_semaphore(&state);
			state.frame_sync_primitives[frame_number].present_semaphore = state.sync_pool.new_semaphore(&state);
			state.frame_sync_primitives[frame_number].render_fence = state.sync_pool.new_fence(&state);
		}

		for (int16_t queue_num = 0; queue_num < static_cast<int16_t>(DeviceQueueType::Num); ++queue_num)
		{
			state.queues[queue_num] = nullptr;
		}
	}

	void VulkanContext::destroy(ExecutionQueue& deletion_queue)
	{
		deletion_queue.flush();

		state.descriptor_set_allocator->destroy(state.get_device());
		state.buffer_allocator->destroy();

		for (int16_t frame_number = MAX_BUFFERED_FRAMES - 1; frame_number >= 0; --frame_number)
		{
			state.sync_pool.release_fence(&state, state.frame_sync_primitives[frame_number].render_fence);
			state.sync_pool.release_semaphore(&state, state.frame_sync_primitives[frame_number].present_semaphore);
			state.sync_pool.release_semaphore(&state, state.frame_sync_primitives[frame_number].render_semaphore);
		}


		vkDestroySurfaceKHR(state.instance, state.surface, nullptr);
		vkDestroyDevice(state.get_device(), nullptr);
		vkb::destroy_debug_utils_messenger(state.instance, state.debug_messenger);
		vkDestroyInstance(state.instance, nullptr);
	}

	void VulkanContext::wait_for_gpu()
	{
		const int16_t current_buffered_frame = get_buffered_frame_number();
		VK_CHECK(vkWaitForFences(state.get_device(), 1, &state.sync_pool.get_fence(state.frame_sync_primitives[current_buffered_frame].render_fence), true, 1000000000));
		VK_CHECK(vkResetFences(state.get_device(), 1, &state.sync_pool.get_fence(state.frame_sync_primitives[current_buffered_frame].render_fence)));
	}

	void VulkanContext::draw(void* buffer, uint32_t vertex_count, uint32_t instance_count, uint32_t instance_index)
	{
		vkCmdDraw(static_cast<VkCommandBuffer>(buffer), vertex_count, instance_count, 0, instance_index);
	}

	void VulkanContext::draw_indexed(void* buffer, uint32_t index_count, uint32_t instance_count, uint32_t instance_index /*= 0*/)
	{
		vkCmdDrawIndexed(static_cast<VkCommandBuffer>(buffer), index_count, instance_count, 0, 0, instance_index);
	}

	void VulkanContext::draw_indexed_indirect(void* buffer, class Buffer* indirect_buffer, uint32_t draw_count, uint32_t draw_first /*= 0*/)
	{
		const uint32_t stride = sizeof(VulkanGPUIndirectObject);
		const VkDeviceSize indirect_offset = draw_first * stride;
		vkCmdDrawIndexedIndirect(static_cast<VkCommandBuffer>(buffer), static_cast<VkBuffer>(indirect_buffer->get()), indirect_offset, draw_count, stride);
	}

	void VulkanContext::dispatch_compute(void* buffer, uint32_t count_x, uint32_t count_y, uint32_t count_z)
	{
		vkCmdDispatch(static_cast<VkCommandBuffer>(buffer), count_x, count_y, count_z);
	}

	void VulkanContext::register_command_queue(DeviceQueueType queue_type)
	{
		const int16_t queue_type_idx = static_cast<int16_t>(queue_type);
		if (state.queues[queue_type_idx] == nullptr)
		{
			state.queues[queue_type_idx] = GraphicsCommandQueueFactory::create(&state, queue_type);
		}
	}

	void VulkanContext::destroy_command_queue(DeviceQueueType queue_type)
	{
		const int16_t queue_type_idx = static_cast<int16_t>(queue_type);
		if (state.queues[queue_type_idx] != nullptr)
		{
			state.queues[queue_type_idx]->destroy(&state);
		}
	}

	CommandQueue* VulkanContext::get_command_queue(DeviceQueueType queue_type)
	{
		return state.queues[static_cast<int16_t>(queue_type)];
	}

	void VulkanContext::push_constants(void* buffer, PipelineStateID pipeline_state, const PushConstantPipelineData& push_constant_data)
	{
		VkCommandBuffer command_buffer = static_cast<VkCommandBuffer>(buffer);
		PipelineState* const pso = CACHE_FETCH(PipelineState, pipeline_state);
		assert(pso != nullptr && "Cannot push constants to a null pipeline state");

		VkPipelineLayout pipeline_layout = static_cast<VkPipelineLayout>(CACHE_FETCH(ShaderPipelineLayout, pso->get_state_data().layout)->get_data());
		assert(pipeline_layout != nullptr && "Cannot push constants to a pipeline state with a null pipeline layout object");

		vkCmdPushConstants(command_buffer, pipeline_layout, VK_FROM_SUNSET_SHADER_STAGE_TYPE(push_constant_data.shader_stage), push_constant_data.offset, static_cast<uint32_t>(push_constant_data.size), push_constant_data.data);
	}

	void VulkanContext::push_descriptor_writes(const std::vector<DescriptorWrite>& descriptor_writes)
	{
		std::vector<VkDescriptorBufferInfo> vk_buffer_infos;
		std::vector<VkDescriptorImageInfo> vk_image_infos;
		std::vector<VkWriteDescriptorSet> vk_writes;

		vk_buffer_infos.reserve(descriptor_writes.size() * MAX_DESCRIPTOR_BINDINGS);
		vk_image_infos.reserve(descriptor_writes.size() * MAX_DESCRIPTOR_BINDINGS);

		for (const DescriptorWrite& write : descriptor_writes)
		{
			if (write.type == DescriptorType::Image)
			{
				Image* const image = static_cast<Image*>(write.buffer_desc.buffer);

				uint32_t image_infos_write_start = vk_image_infos.size();
				for (int i = 0; i < write.count; ++i)
				{
					VkDescriptorImageInfo& image_info = vk_image_infos.emplace_back();
					image_info.sampler = static_cast<VkSampler>(image->get_sampler());
					image_info.imageView = static_cast<VkImageView>(image->get_image_view());
					image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
				auto data_it = vk_image_infos.begin() + image_infos_write_start;

				VkWriteDescriptorSet& new_vk_write = vk_writes.emplace_back();
				new_vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				new_vk_write.pNext = nullptr;
				new_vk_write.descriptorCount = write.count;
				new_vk_write.descriptorType = VK_FROM_SUNSET_DESCRIPTOR_TYPE(write.type);
				new_vk_write.pImageInfo = &(*data_it);
				new_vk_write.dstBinding = write.slot;
				new_vk_write.dstSet = static_cast<VkDescriptorSet>(write.set->get());
				new_vk_write.dstArrayElement = write.array_index;
			}
			else
			{
				uint32_t buffer_infos_write_start = vk_buffer_infos.size();
				for (uint32_t i = 0; i < write.count; ++i)
				{
					VkDescriptorBufferInfo& buffer_info = vk_buffer_infos.emplace_back();
					buffer_info.buffer = static_cast<VkBuffer>(write.buffer_desc.buffer);
					buffer_info.offset = 0;
					buffer_info.range = write.buffer_desc.buffer_range;
				}
				auto data_it = vk_buffer_infos.begin() + buffer_infos_write_start;

				VkWriteDescriptorSet& new_vk_write = vk_writes.emplace_back();
				new_vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				new_vk_write.pNext = nullptr;
				new_vk_write.descriptorCount = write.count;
				new_vk_write.descriptorType = VK_FROM_SUNSET_DESCRIPTOR_TYPE(write.type);
				new_vk_write.pBufferInfo = &(*data_it);
				new_vk_write.dstBinding = write.slot;
				new_vk_write.dstSet = static_cast<VkDescriptorSet>(write.set->get());
				new_vk_write.dstArrayElement = write.array_index;
			}
		}

		vkUpdateDescriptorSets(state.get_device(), static_cast<uint32_t>(vk_writes.size()), vk_writes.data(), 0, nullptr);
	}

	size_t VulkanContext::get_min_ubo_offset_alignment()
	{
		return state.device.physical_device.properties.limits.minUniformBufferOffsetAlignment;
	}

	void VulkanContext::update_indirect_draw_command(void* commands, uint32_t command_index, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance)
	{
		VulkanGPUIndirectObject* vk_commands = static_cast<VulkanGPUIndirectObject*>(commands);
		vk_commands[command_index].indirect_command.indexCount = index_count;
		vk_commands[command_index].indirect_command.firstIndex = first_index;
		vk_commands[command_index].indirect_command.instanceCount = instance_count;
		vk_commands[command_index].indirect_command.firstInstance = first_instance;
	}

	Sunset::ShaderLayoutID VulkanContext::derive_layout_for_shader_stages(class GraphicsContext* const gfx_context, const std::vector<PipelineShaderStage>& stages, std::vector<DescriptorLayoutID>& out_descriptor_layouts)
	{
		std::vector<uint64_t> set_binding_pairs;
		std::vector<DescriptorBinding> all_bindings;
		std::vector<PushConstantPipelineData> push_constant_data;

		size_t max_total_sets{ 0 };

		for (const PipelineShaderStage& stage : stages)
		{
			Shader* const shader = CACHE_FETCH(Shader, stage.shader_module);

			SpvReflectShaderModule spv_module;
			SpvReflectResult result = spvReflectCreateShaderModule(shader->get_code_size(), shader->get_code(), &spv_module);

			uint32_t count = 0;
			result = spvReflectEnumerateDescriptorSets(&spv_module, &count, nullptr);
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			std::vector<SpvReflectDescriptorSet*> sets(count);
			result = spvReflectEnumerateDescriptorSets(&spv_module, &count, sets.data());
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			max_total_sets = std::max(max_total_sets, sets.size());

			for (size_t s = 0; s < sets.size(); ++s)
			{
				const SpvReflectDescriptorSet& reflected_set = *(sets[s]);

				for (int b = 0; b < reflected_set.binding_count; ++b)
				{
					const SpvReflectDescriptorBinding& reflected_binding = *(reflected_set.bindings[b]);

					DescriptorBinding& binding = all_bindings.emplace_back();
					binding.slot = reflected_binding.binding;
					binding.count = reflected_binding.count > 0 ? reflected_binding.count : MAX_DESCRIPTOR_BINDINGS - 1;
					binding.type = SUNSET_FROM_VK_DESCRIPTOR_TYPE(static_cast<VkDescriptorType>(reflected_binding.descriptor_type));
					binding.pipeline_stages = stage.stage_type;
					binding.b_supports_bindless = static_cast<bool>(reflected_binding.count ^ 1);

					set_binding_pairs.push_back(((uint64_t)reflected_set.set << 32) | (uint32_t)(all_bindings.size() - 1));
				}
			}

			result = spvReflectEnumeratePushConstantBlocks(&spv_module, &count, NULL);
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			std::vector<SpvReflectBlockVariable*> push_constants(count);
			result = spvReflectEnumeratePushConstantBlocks(&spv_module, &count, push_constants.data());
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			if (count > 0)
			{
				PushConstantPipelineData& pc_data = push_constant_data.emplace_back();
				pc_data.offset = push_constants[0]->offset;
				pc_data.size = push_constants[0]->size;
				pc_data.shader_stage = stage.stage_type;
			}

			spvReflectDestroyShaderModule(&spv_module);
		}

		std::sort(set_binding_pairs.begin(), set_binding_pairs.end());

		{
			out_descriptor_layouts.clear();
			out_descriptor_layouts.resize(max_total_sets, 0);

			uint32_t previous_set_number{ std::numeric_limits<uint32_t>::max() };
			std::unordered_map<uint32_t, DescriptorBinding> unique_bindings;

			auto build_unique_bindings_into_layout = [&](uint32_t set_number)
			{
				std::vector<DescriptorBinding> bindings_to_build;

				for (auto pair : unique_bindings)
				{
					bindings_to_build.emplace_back(pair.second);
				}

				std::sort(bindings_to_build.begin(), bindings_to_build.end(),
					[](const DescriptorBinding& first, const DescriptorBinding& second)
					{
						return first.slot < second.slot;
					}
				);

				out_descriptor_layouts[set_number] = DescriptorLayoutFactory::create(gfx_context, bindings_to_build);

				unique_bindings.clear();
			};

			for (uint64_t set_binding_pair : set_binding_pairs)
			{
				uint32_t set_number = set_binding_pair >> 32;
				uint32_t binding_index = set_binding_pair;

				if (previous_set_number == std::numeric_limits<uint32_t>::max())
				{
					previous_set_number = set_number;
				}

				if (set_number != previous_set_number)
				{
					build_unique_bindings_into_layout(previous_set_number);
					previous_set_number = set_number;
				}

				DescriptorBinding& binding = all_bindings[binding_index];
				if (unique_bindings.find(binding.slot) != unique_bindings.end())
				{
					unique_bindings[binding.slot].pipeline_stages |= binding.pipeline_stages;
				}
				else
				{
					unique_bindings[binding.slot] = binding;
				}
			}

			build_unique_bindings_into_layout(previous_set_number);
		}

		return ShaderPipelineLayoutFactory::create(gfx_context, push_constant_data, out_descriptor_layouts);
	}

	void create_surface(VulkanContext* const vulkan_context, Window* const window)
	{
#if USE_SDL_WINDOWING
		SDL_Vulkan_CreateSurface(static_cast<SDL_Window*>(window->get_window_handle()), vulkan_context->state.instance, &vulkan_context->state.surface);
#endif
	}
}
