cmake_minimum_required (VERSION VERSION 3.20)

set(SUNSET_LIB Sunset)
project(${SUNSET_LIB})

set(ENGINE_DIR "${PROJECT_SOURCE_DIR}/engine")

set(CMAKE_CXX_STANDARD 20)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ENGINE_DIR}/bin")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

function(set_target_third_party target)
  if(MSVC_IDE)
    set_target_properties(${target} PROPERTIES FOLDER ThirdParty)
  endif()
endfunction()

include(FetchContent)

set(BENCHMARK_ENABLE_TESTING off)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Fetch GoogleTest and GoogleBenchmark
FetchContent_Declare(googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG main)
FetchContent_Declare(googlebenchmark
	GIT_REPOSITORY https://github.com/google/benchmark.git
	GIT_TAG main)
FetchContent_MakeAvailable(googletest googlebenchmark)

set_target_third_party(gtest)
set_target_third_party(gtest_main)
set_target_third_party(gmock)
set_target_third_party(gmock_main)
set_target_third_party(benchmark)
set_target_third_party(benchmark_main)

find_package(Vulkan REQUIRED)

# Fetch SPIRV-Headers
FetchContent_Declare(
  spirv-headers
  GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
  GIT_TAG        main
)
FetchContent_GetProperties(spirv-headers)
if(NOT spirv-headers_POPULATED)
  FetchContent_Populate(spirv-headers)

  add_subdirectory(${spirv-headers_SOURCE_DIR} ${spirv-headers_BINARY_DIR})
endif()

# Fetch SPIRV-Tools
FetchContent_Declare(
  spirv-tools
  GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
  GIT_TAG        main
)
FetchContent_GetProperties(spirv-tools)
if(NOT spirv-tools_POPULATED)
  FetchContent_Populate(spirv-tools)

  set(SPIRV-Headers_SOURCE_DIR ${spirv-headers_SOURCE_DIR})
  set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
  set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)

  add_subdirectory(${spirv-tools_SOURCE_DIR} ${spirv-tools_BINARY_DIR})

  set_target_third_party(spirv-tools-pkg-config)
  set_target_third_party(spirv-tools-shared-pkg-config)
  set_target_third_party(core_tables)
  set_target_third_party(enum_string_mapping)
  set_target_third_party(extinst_tables)
  set_target_third_party(spirv-tools-vimsyntax)
  set_target_third_party(SPIRV-Tools-static)
  set_target_third_party(SPIRV-Tools-shared)
  set_target_third_party(SPIRV-Tools-reduce)
  set_target_third_party(SPIRV-Tools-opt)
  set_target_third_party(SPIRV-Tools-lint)
  set_target_third_party(SPIRV-Tools-link)
  set_target_third_party(SPIRV-Tools-diff)
  set_target_third_party(spirv-tools-build-version)
  set_target_third_party(spirv-tools-header-DebugInfo)
  set_target_third_party(spirv-tools-header-NonSemanticShaderDebugInfo100)
  set_target_third_party(spirv-tools-header-OpenCLDebugInfo100)
  set_target_third_party(spv-tools-cldi100)
  set_target_third_party(spv-tools-clspvreflection)
  set_target_third_party(spv-tools-debuginfo)
  set_target_third_party(spv-tools-shdi100)
  set_target_third_party(spv-tools-spv-amd-gs)
  set_target_third_party(spv-tools-spv-amd-sb)
  set_target_third_party(spv-tools-spv-amd-sevp)
  set_target_third_party(spv-tools-spv-amd-stm)

  target_compile_options(SPIRV-Tools-opt PRIVATE /Wv:18)
  set_target_properties(SPIRV-Tools-opt PROPERTIES CXX_STANDARD 17)
endif()

# Fetch glslang
FetchContent_Declare(
  glslang
  GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
  GIT_TAG        main
)
FetchContent_GetProperties(glslang)
if(NOT glslang_POPULATED)
  FetchContent_Populate(glslang)
  set(ENABLE_CTEST OFF CACHE BOOL "" FORCE)

  add_subdirectory(${glslang_SOURCE_DIR} ${glslang_BINARY_DIR})

  if (TARGET glslangValidator)
    set_target_third_party(glslangValidator)
  endif()
  set_target_third_party(spirv-remap)
  set_target_third_party(HLSL)
  set_target_third_party(GenericCodeGen)
  set_target_third_party(glslang)
  set_target_third_party(glslang-default-resource-limits)
  set_target_third_party(MachineIndependent)
  set_target_third_party(OGLCompiler)
  set_target_third_party(OSDependent)
  set_target_third_party(SPIRV)
  set_target_third_party(SPVRemapper)
endif()

# Fetch Shaderc
set(SHADERC_ENABLE_SHARED_CRT ON CACHE BOOL "" FORCE)

FetchContent_Declare(
  shaderc
  GIT_REPOSITORY https://github.com/google/shaderc.git
  GIT_TAG        main
)
FetchContent_GetProperties(shaderc)
if(NOT shaderc_POPULATED)
  FetchContent_Populate(shaderc)

  set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
  set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)
  set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
  set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "" FORCE)
  set(SPIRV-Headers_SOURCE_DIR ${spirv-headers_SOURCE_DIR})
  set(SPIRV-Tools_SOURCE_DIR ${spirv-tools_SOURCE_DIR})

  add_subdirectory(${shaderc_SOURCE_DIR} ${shaderc_BINARY_DIR})

  set_target_third_party(add-copyright)
  set_target_third_party(build-version)
  set_target_third_party(glslc)
  set_target_third_party(glslc_exe)
  set_target_third_party(shaderc)
  set_target_third_party(shaderc_combined_genfile)
  set_target_third_party(shaderc_combined-pkg-config)
  set_target_third_party(shaderc_shared)
  set_target_third_party(shaderc_static-pkg-config)
  set_target_third_party(shaderc_util)
  set_target_third_party(shaderc-pkg-config)
  set_target_third_party(testdata)
endif()

# Fetch Jolt
FetchContent_Declare(
    JoltPhysics
    GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
    GIT_TAG master 
    SOURCE_SUBDIR "Build"
)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(JoltPhysics)

set_target_third_party(Jolt)

if (MSVC)
    add_definitions(/MP)
endif()

add_subdirectory("${ENGINE_DIR}")
