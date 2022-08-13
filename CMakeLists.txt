cmake_minimum_required (VERSION 3.8)

set(SUNSET_LIB Sunset)
project(${SUNSET_LIB})

set(ENGINE_DIR "${PROJECT_SOURCE_DIR}/engine")

set(CMAKE_CXX_STANDARD 20)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ENGINE_DIR}/bin")

find_package(Vulkan REQUIRED)

add_subdirectory("${ENGINE_DIR}")

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
