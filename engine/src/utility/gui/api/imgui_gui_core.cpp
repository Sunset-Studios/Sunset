#include <utility/gui/api/imgui_gui_core.h>
#include <graphics/graphics_context.h>
#include <graphics/command_queue.h>
#include <graphics/renderer.h>
#include <graphics/render_pass.h>
#include <window/window.h>

#include <vk_types.h>
#include <vk_initializers.h>

#include <VkBootstrap.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>

#include <SDL.h>

namespace Sunset
{
	void ImGUICore::initialize(class GraphicsContext* gfx_context, class Window* const window)
	{
		if (!b_initialized)
		{
			b_initialized = true;

			VulkanContextState* context_state = static_cast<VulkanContextState*>(gfx_context->get_state());
			assert(context_state != nullptr);

			VkDescriptorPool imgui_pool;
			{
				VkDescriptorPoolSize pool_sizes[] =
				{
					{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
					{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
					{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
					{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
				};

				VkDescriptorPoolCreateInfo pool_info = {};
				pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
				pool_info.maxSets = 1000;
				pool_info.poolSizeCount = std::size(pool_sizes);
				pool_info.pPoolSizes = pool_sizes;

				VK_CHECK(vkCreateDescriptorPool(context_state->get_device(), &pool_info, nullptr, &imgui_pool));
			}

			ImGui::CreateContext();

			ImGui_ImplSDL2_InitForVulkan(static_cast<SDL_Window*>(window->get_window_handle()));

			{
				ImGui_ImplVulkan_InitInfo init_info = {};
				init_info.Instance = context_state->instance;
				init_info.PhysicalDevice = context_state->get_gpu();
				init_info.Device = context_state->get_device();
				init_info.Queue = context_state->device.get_queue(vkb::QueueType::graphics).value();
				init_info.DescriptorPool = imgui_pool;
				init_info.MinImageCount = 3;
				init_info.ImageCount = 3;
				init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

				VulkanRenderPassData* render_pass_data = static_cast<VulkanRenderPassData*>(Renderer::get()->master_pass()->get_data());
				ImGui_ImplVulkan_Init(&init_info, render_pass_data->render_pass);
			}

			gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(gfx_context, [](void* command_buffer)
			{
				ImGui_ImplVulkan_CreateFontsTexture(static_cast<VkCommandBuffer>(command_buffer));
			});

			ImGui_ImplVulkan_DestroyFontUploadObjects();

			gfx_context->add_resource_deletion_execution([=]()
			{
				vkDestroyDescriptorPool(context_state->get_device(), imgui_pool, nullptr);
				ImGui_ImplVulkan_Shutdown();
			});
		}
	}

	void ImGUICore::new_frame(class Window* const window)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame(static_cast<SDL_Window*>(window->get_window_handle()));
		ImGui::NewFrame();
	}

	void ImGUICore::poll_events()
	{
		ImGui_ImplSDL2_ProcessEvent(&sdl_event);
	}

	void ImGUICore::begin_draw()
	{
		ImGui::Render();
	}

	void ImGUICore::end_draw(void* command_buffer)
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), static_cast<VkCommandBuffer>(command_buffer));
	}
}
