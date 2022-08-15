#pragma once

#define ENGINE_NAME "Sunset"

#define USE_VULKAN_GRAPHICS 1
#define USE_SDL_WINDOWING 1

#if USE_VULKAN_GRAPHICS
#include <graphics/api/vulkan/vk_context.h>
#include <graphics/api/vulkan/vk_swapchain.h>
#include <graphics/api/vulkan/vk_command_queue.h>
#include <graphics/api/vulkan/vk_render_pass.h>
#include <graphics/api/vulkan/vk_framebuffer.h>
#include <graphics/api/vulkan/vk_shader.h>
#include <graphics/api/vulkan/vk_pipeline_state.h>
#include <graphics/api/vulkan/vk_shader_pipeline_layout.h>
#endif

#if USE_SDL_WINDOWING
#include <vendor/sdl_context.h>
#endif

#include <glm/glm.hpp>

#include <graphics/viewport.h>