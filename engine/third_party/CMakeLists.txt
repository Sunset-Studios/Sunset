find_package(Vulkan REQUIRED)

option( TRACY_ENABLE "" ON)
option( TRACY_ON_DEMAND "" ON)
option( TRACY_ONLY_LOCALHOST "" ON)
add_subdirectory(tracy)

set_target_third_party(TracyClient)

add_library(vkbootstrap STATIC)

set_target_third_party(vkbootstrap)

add_library(glm INTERFACE)
add_library(vma INTERFACE)

add_library(stb_image INTERFACE)
add_library(json INTERFACE)

add_library(tinyobjloader STATIC)

set_target_third_party(tinyobjloader)

add_library(lz4 STATIC)

set_target_third_party(lz4)

target_sources(vkbootstrap PRIVATE 
    vkbootstrap/VkBootstrap.h
    vkbootstrap/VkBootstrap.cpp
    )

target_include_directories(vkbootstrap PUBLIC vkbootstrap)
target_link_libraries(vkbootstrap PUBLIC Vulkan::Vulkan $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)

#both vma and glm and header only libs so we only need the include path
target_include_directories(vma INTERFACE vma)
target_include_directories(glm INTERFACE glm)

target_sources(tinyobjloader PRIVATE 
    tinyobjloader/tiny_obj_loader.h
    tinyobjloader/tiny_obj_loader.cc
    )

target_include_directories(tinyobjloader PUBLIC tinyobjloader)

target_sources(lz4 PRIVATE
    lz4/lz4.h
    lz4/lz4.c)

target_include_directories(lz4 PUBLIC lz4)

add_library(sdl2 INTERFACE)
set(sdl2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdl2" CACHE PATH "Path to SDL2")

if (WIN32)
target_include_directories(sdl2 INTERFACE ${sdl2_DIR}/include ${sdl2_image_DIR}/include)
target_link_directories(sdl2 INTERFACE ${sdl2_DIR}/lib/x64 ${sdl2_image_DIR}/lib/x64)
elseif (UNIX)
target_include_directories(sdl2 INTERFACE ${sdl2_DIR}/include/SDL2 ${sdl2_image_DIR}/include/SDL2)
target_link_directories(sdl2 INTERFACE ${sdl2_DIR}/lib ${sdl2_image_DIR}/lib)
endif()

target_link_libraries(sdl2 INTERFACE SDL2 SDL2main)

add_library(imgui STATIC)

set_target_third_party(imgui)

target_include_directories(imgui PUBLIC imgui)

target_sources(imgui PRIVATE 
    imgui/imgui.h
    imgui/imgui.cpp

    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp

    imgui/imgui_impl_vulkan.cpp
    imgui/imgui_impl_sdl.cpp
    )

target_link_libraries(imgui PUBLIC Vulkan::Vulkan sdl2)

target_include_directories(stb_image INTERFACE stb_image)
target_include_directories(json INTERFACE json)

add_library(spirv_reflect STATIC)

set_target_third_party(spirv_reflect)

target_include_directories(spirv_reflect PUBLIC spirv)

target_sources(spirv_reflect PRIVATE 
    spirv/spirv_reflect.h
    spirv/spirv_reflect.c
)