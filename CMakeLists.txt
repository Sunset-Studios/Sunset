cmake_minimum_required (VERSION VERSION 3.20)

set(SUNSET_LIB Sunset)
project(${SUNSET_LIB})

set(ENGINE_DIR "${PROJECT_SOURCE_DIR}/engine")

set(CMAKE_CXX_STANDARD 20)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ENGINE_DIR}/bin")

include(FetchContent)

set(BENCHMARK_ENABLE_TESTING off)
FetchContent_Declare(googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG main)
FetchContent_Declare(googlebenchmark
	GIT_REPOSITORY https://github.com/google/benchmark.git
	GIT_TAG main)
FetchContent_MakeAvailable(googletest googlebenchmark)

find_package(Vulkan REQUIRED)

add_subdirectory("${ENGINE_DIR}")

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
