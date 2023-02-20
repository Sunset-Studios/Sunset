#pragma once

#include <utility/execution_queue.h>
#include <graphics/pipeline_types.h>
#include <graphics/pipeline_types.h>
#include <graphics/command_queue_types.h>

#include <vk_types.h>
#include <vk_initializers.h>
#include <vk_sync.h>

#include <VkBootstrap.h>

namespace Sunset
{
	struct VulkanFrameSyncPrimitives
	{
		VulkanSemaphoreHandle present_semaphore{ -1 };
		VulkanSemaphoreHandle render_semaphore{ -1 };
		VulkanFenceHandle render_fence{ -1 };
	};

	struct VulkanContextState
	{
		public:
			VkInstance instance{ nullptr };
			vkb::Device device;
			vkb::PhysicalDevice physical_device;
			VkDebugUtilsMessengerEXT debug_messenger{ nullptr };
			VkSurfaceKHR surface{ nullptr };
			VulkanSyncPool sync_pool;
			VulkanFrameSyncPrimitives frame_sync_primitives[MAX_BUFFERED_FRAMES];
			uint32_t frame_number{ 0 };
			class Window* window{ nullptr };
			class CommandQueue* queues[static_cast<int16_t>(DeviceQueueType::Num)];
			class BufferAllocator* buffer_allocator{ nullptr };
			class DescriptorSetAllocator* descriptor_set_allocator{ nullptr };
			bool supports_bindless{ false };

		public:
			VulkanContextState() = default;
			VulkanContextState(VulkanContextState&& other) = delete;
			VulkanContextState(const VulkanContextState& other) = delete;
			VulkanContextState& operator=(const VulkanContextState& other) = delete;

			VkDevice get_device()
			{
				return device.device;
			}

			VkPhysicalDevice get_gpu()
			{
				return physical_device.physical_device;
			}
	};

	class VulkanContext
	{
		public:
			VulkanContext() = default;
			~VulkanContext() = default;

		public:
			void initialize(class Window* const window);
			void destroy(ExecutionQueue& deletion_queue);
			void wait_for_gpu();
			void draw(void* buffer, uint32_t vertex_count, uint32_t instance_count, uint32_t instance_index = 0);
			void draw_indexed(void* buffer, uint32_t index_count, uint32_t instance_count, uint32_t instance_index = 0);
			void draw_indexed_indirect(void* buffer, class Buffer* indirect_buffer, uint32_t draw_count, uint32_t draw_first = 0);
			void dispatch_compute(void* buffer, uint32_t count_x, uint32_t count_y, uint32_t count_z);

			void* get_state()
			{
				return &state;
			}

			class Window* get_window()
			{
				return state.window;
			}

			void set_buffer_allocator(class BufferAllocator* allocator)
			{
				state.buffer_allocator = allocator;
			}

			void set_descriptor_set_allocator(class DescriptorSetAllocator* allocator)
			{
				state.descriptor_set_allocator = allocator;
			}

			class BufferAllocator* get_buffer_allocator()
			{
				return state.buffer_allocator;
			}

			class DescriptorSetAllocator* get_descriptor_set_allocator()
			{
				return state.descriptor_set_allocator;
			}

			void register_command_queue(DeviceQueueType queue_type);
			void destroy_command_queue(DeviceQueueType queue_type);
			class CommandQueue* get_command_queue(DeviceQueueType queue_type);

			uint32_t get_frame_number() const
			{
				return state.frame_number;
			}

			uint16_t get_buffered_frame_number() const
			{
				return state.frame_number % MAX_BUFFERED_FRAMES;
			}

			void advance_frame()
			{
				++state.frame_number;
			}

			void push_constants(void* buffer, PipelineStateID pipeline_state, const PushConstantPipelineData& push_constant_data);
			void push_descriptor_writes(const std::vector<DescriptorWrite>& descriptor_writes);
			size_t get_min_ubo_offset_alignment();
			void update_indirect_draw_command(void* commands, uint32_t command_index, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint64_t object_id, uint32_t batch_id);
			ShaderLayoutID derive_layout_for_shader_stages(class GraphicsContext* const gfx_context, const std::vector<PipelineShaderStage>& stages);

		public:
			VulkanContextState state;
	};

	void create_surface(VulkanContext* const vulkan_context, class Window* const window);
}