MACRO(HEADER_DIRECTORIES_APPEND append_list base_path)
  file(GLOB_RECURSE header_files
    RELATIVE "${base_path}"
    "${base_path}/*.h"
    "${base_path}/*.hpp"
  )

  set(header_directories "")
  foreach(header_file ${header_files})
    get_filename_component(dir_path "${header_file}" PATH)
    set(header_directories ${header_directories} "${base_path}/${dir_path}")
  endforeach()

  list(REMOVE_DUPLICATES header_directories)
  list(APPEND ${append_list} ${header_directories})
  set(${append_list} "${${append_list}}" PARENT_SCOPE)
ENDMACRO()

find_package(Vulkan REQUIRED)

option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" ON)
option(TRACY_ONLY_LOCALHOST "" ON)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tracy)

add_library(vkbootstrap STATIC)
add_library(glm INTERFACE)
add_library(vma INTERFACE)
add_library(stb_image INTERFACE)
add_library(json INTERFACE)
add_library(tinyobjloader STATIC)
add_library(lz4 STATIC)
add_library(sdl2 INTERFACE)
add_library(imgui STATIC)
add_library(spirv_reflect STATIC)
add_library(atomic_queue INTERFACE)
add_library(parallel_hashmap INTERFACE)
add_library(open_fbx STATIC)

set_target_third_party(TracyClient)
set_target_third_party(vkbootstrap)
set_target_third_party(tinyobjloader)
set_target_third_party(lz4)
set_target_third_party(imgui)
set_target_third_party(spirv_reflect)
set_target_third_party(atomic_queue)
set_target_third_party(parallel_hashmap)
set_target_third_party(open_fbx)

target_sources(vkbootstrap PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/vkbootstrap/VkBootstrap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vkbootstrap/VkBootstrap.cpp
    )
target_include_directories(vkbootstrap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vkbootstrap)
target_link_libraries(vkbootstrap PUBLIC Vulkan::Vulkan $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)

#both vma and glm and header only libs so we only need the include path
target_include_directories(vma INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vma)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glm)

target_sources(tinyobjloader PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader/tiny_obj_loader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader/tiny_obj_loader.cc
    )
target_include_directories(tinyobjloader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader)

target_sources(lz4 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lz4.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lz4.c)
target_include_directories(lz4 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lz4)

set(sdl2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdl2" CACHE PATH "Path to SDL2")
if (WIN32)
target_include_directories(sdl2 INTERFACE ${sdl2_DIR}/include)
target_link_directories(sdl2 INTERFACE ${sdl2_DIR}/lib/x64)
elseif (UNIX)
target_include_directories(sdl2 INTERFACE ${sdl2_DIR}/include/SDL2)
target_link_directories(sdl2 INTERFACE ${sdl2_DIR}/lib)
endif()
target_link_libraries(sdl2 INTERFACE SDL2 SDL2main)

target_sources(imgui PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_vulkan.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_sdl.cpp
    )
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
target_link_libraries(imgui PUBLIC Vulkan::Vulkan sdl2)

target_include_directories(stb_image INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb_image)

target_include_directories(json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/json)

target_include_directories(spirv_reflect PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/spirv)
target_sources(spirv_reflect PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv/spirv_reflect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv/spirv_reflect.c
)

target_include_directories(atomic_queue INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/atomic_queue)
target_include_directories(parallel_hashmap INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/parallel_hashmap)

target_include_directories(open_fbx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/openfbx)
target_sources(open_fbx PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/openfbx/ofbx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openfbx/ofbx.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openfbx/miniz.c
    ${CMAKE_CURRENT_SOURCE_DIR}/openfbx/miniz.h
)

list(APPEND INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

HEADER_DIRECTORIES_APPEND(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tracy/public")
HEADER_DIRECTORIES_APPEND(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/vkbootstrap")
HEADER_DIRECTORIES_APPEND(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/glm")
HEADER_DIRECTORIES_APPEND(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/vma")
HEADER_DIRECTORIES_APPEND(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/stb_image")
HEADER_DIRECTORIES_APPEND(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/json")
HEADER_DIRECTORIES_APPEND(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tiny_obj_loader")
HEADER_DIRECTORIES_APPEND(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/lz4")
HEADER_DIRECTORIES_APPEND(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
HEADER_DIRECTORIES_APPEND(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/spirv")
HEADER_DIRECTORIES_APPEND(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/atomic_queue")
HEADER_DIRECTORIES_APPEND(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/parallel_hashmap")
HEADER_DIRECTORIES_APPEND(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/openfbx")
HEADER_DIRECTORIES_APPEND(INCLUDE_DIRS "${sdl2_DIR}/include")
