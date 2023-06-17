#pragma once

#include <minimal.h>

#ifndef USE_VULKAN_GRAPHICS
#define USE_VULKAN_GRAPHICS 1
#endif

#ifndef USE_SDL_WINDOWING
#define USE_SDL_WINDOWING 1
#endif

#ifndef USE_JOLT_PHYSICS
#define USE_JOLT_PHYSICS 1
#endif

#if USE_VULKAN_GRAPHICS
#include <graphics/api/vulkan/vk_context.h>
#include <graphics/api/vulkan/vk_swapchain.h>
#include <graphics/api/vulkan/vk_command_queue.h>
#include <graphics/api/vulkan/vk_render_pass.h>
#include <graphics/api/vulkan/vk_framebuffer.h>
#include <graphics/api/vulkan/vk_shader.h>
#include <graphics/api/vulkan/vk_pipeline_state.h>
#include <graphics/api/vulkan/vk_shader_pipeline_layout.h>
#include <graphics/api/vulkan/vk_buffer.h>
#include <graphics/api/vulkan/vk_image.h>
#include <graphics/api/vulkan/vk_descriptor.h>
#include <graphics/api/vulkan/vk_barrier_batcher.h>
#endif

#if USE_SDL_WINDOWING
#include <vendor/sdl_context.h>
#endif

#if defined USE_VULKAN_GRAPHICS && defined USE_SDL_WINDOWING
#include <utility/gui/api/imgui_gui_core.h>
#endif

#if USE_JOLT_PHYSICS
#include <physics/api/jolt/jolt_context.h>
#include <physics/api/jolt/jolt_body.h>
#endif
