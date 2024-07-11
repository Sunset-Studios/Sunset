cmake_minimum_required (VERSION VERSION 3.20)

set(SUNSET_LIB Sunset)
project(${SUNSET_LIB})

set(ENGINE_DIR "${PROJECT_SOURCE_DIR}/engine")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ENGINE_DIR}/bin")
cmake_policy(SET CMP0135 NEW)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSVC)
    add_definitions(/MP)
endif()

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
  set_target_third_party(GenericCodeGen)
  set_target_third_party(glslang)
  set_target_third_party(glslang-default-resource-limits)
  set_target_third_party(MachineIndependent)
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
  set_target_third_party(shaderc_combined-pkg-config)
  set_target_third_party(shaderc_shared)
  set_target_third_party(shaderc_combined)
  set_target_third_party(shaderc_static-pkg-config)
  set_target_third_party(shaderc_util)
  set_target_third_party(shaderc-pkg-config)
  set_target_third_party(testdata)
endif()

# Fetch VMA
FetchContent_Declare(
  vma
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG        master
)
FetchContent_GetProperties(vma)
if(NOT vma_POPULATED)
  FetchContent_Populate(vma)
  add_library(vma INTERFACE)
  target_include_directories(vma INTERFACE ${vma_SOURCE_DIR}/include)
endif()

# Fetch vkboostrap
FetchContent_Declare(
  vkbootstrap
  GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git
  GIT_TAG        main
)
FetchContent_GetProperties(vkbootstrap)
if(NOT vkbootstrap_POPULATED)
  FetchContent_Populate(vkbootstrap)
  add_library(vkbootstrap STATIC ${vkbootstrap_SOURCE_DIR}/src/VkBootstrap.cpp)
  target_include_directories(vkbootstrap PUBLIC ${vkbootstrap_SOURCE_DIR}/src)
  target_link_libraries(vkbootstrap PUBLIC Vulkan::Vulkan)
endif()

set_target_third_party(vkbootstrap)

# Fetch GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

set_target_third_party(glm)

# Fetch STB
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
    FetchContent_Populate(stb)
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
endif()

# Fetch OpenFBX
FetchContent_Declare(
  openfbx
  GIT_REPOSITORY https://github.com/nem0/OpenFBX.git
  GIT_TAG        master
)
FetchContent_GetProperties(openfbx)
if(NOT openfbx_POPULATED)
  FetchContent_Populate(openfbx)
  add_library(openfbx STATIC ${openfbx_SOURCE_DIR}/src/ofbx.cpp ${openfbx_SOURCE_DIR}/src/libdeflate.c)
  target_include_directories(openfbx PUBLIC ${openfbx_SOURCE_DIR}/src)
endif()

set_target_third_party(openfbx)

# Fetch json
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        master
)
FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_library(json INTERFACE)
  target_include_directories(json INTERFACE ${json_SOURCE_DIR}/single_include/nlohmann)
endif()

# Fetch tinyobjloader
FetchContent_Declare(
  tinyobjloader
  GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
  GIT_TAG        release
)
FetchContent_GetProperties(tinyobjloader)
if(NOT tinyobjloader_POPULATED)
  FetchContent_Populate(tinyobjloader)
  add_library(tinyobjloader STATIC ${tinyobjloader_SOURCE_DIR}/tiny_obj_loader.cc)
  target_include_directories(tinyobjloader PUBLIC ${tinyobjloader_SOURCE_DIR})
endif()

set_target_third_party(tinyobjloader)

# Fetch lz4
FetchContent_Declare(
  lz4
  GIT_REPOSITORY https://github.com/lz4/lz4.git
  GIT_TAG        release
)
FetchContent_GetProperties(lz4)
if(NOT lz4_POPULATED)
  FetchContent_Populate(lz4)
  add_library(lz4 STATIC ${lz4_SOURCE_DIR}/lib/lz4.c)
  target_include_directories(lz4 PUBLIC ${lz4_SOURCE_DIR}/lib)
endif()

set_target_third_party(lz4)

# Fetch parallel_hashmap
FetchContent_Declare(
  parallel_hashmap
  GIT_REPOSITORY https://github.com/greg7mdp/parallel-hashmap.git
  GIT_TAG        master
)
FetchContent_GetProperties(parallel_hashmap)
if(NOT parallel_hashmap_POPULATED)
  FetchContent_Populate(parallel_hashmap)
  add_library(parallel_hashmap INTERFACE)
  target_include_directories(parallel_hashmap INTERFACE ${parallel_hashmap_SOURCE_DIR}/parallel_hashmap)
endif()

set_target_third_party(parallel_hashmap)

# Fetch atomic_queue
FetchContent_Declare(
  atomic_queue
  GIT_REPOSITORY https://github.com/max0x7ba/atomic_queue.git
  GIT_TAG        master
)
FetchContent_GetProperties(atomic_queue)
if(NOT atomic_queue_POPULATED)
  FetchContent_Populate(atomic_queue)
  add_library(atomic_queue INTERFACE)
  target_include_directories(atomic_queue INTERFACE ${atomic_queue_SOURCE_DIR}/include/atomic_queue)
endif()

set_target_third_party(atomic_queue)

# Fetch spirv_reflect
FetchContent_Declare(
  spirv_reflect
  GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git
  GIT_TAG        main
)
FetchContent_GetProperties(spirv_reflect)
if(NOT spirv_reflect_POPULATED)
  FetchContent_Populate(spirv_reflect)
  add_library(spirv_reflect STATIC ${spirv_reflect_SOURCE_DIR}/spirv_reflect.c)
  target_include_directories(spirv_reflect PUBLIC ${spirv_reflect_SOURCE_DIR})
endif()

set_target_third_party(spirv_reflect)

# Fetch sdl2
FetchContent_Declare(
  sdl2
  URL https://github.com/libsdl-org/SDL/releases/download/release-2.28.5/SDL2-devel-2.28.5-VC.zip
)
FetchContent_GetProperties(sdl2)
if(NOT sdl2_POPULATED)
  FetchContent_Populate(sdl2)
  add_library(sdl2 INTERFACE)
  if (WIN32)
    target_include_directories(sdl2 INTERFACE ${sdl2_SOURCE_DIR}/include)
    target_link_directories(sdl2 INTERFACE ${sdl2_SOURCE_DIR}/lib/x64)
  elseif (UNIX)
    target_include_directories(sdl2 INTERFACE ${sdl2_SOURCE_DIR}/include/SDL2)
    target_link_directories(sdl2 INTERFACE ${sdl2_SOURCE_DIR}/lib)
  endif()
  target_link_libraries(sdl2 INTERFACE SDL2 SDL2main)
endif()

set_target_third_party(sdl2)

# Fetch imgui
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        master
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
  add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp)
  target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
  target_link_libraries(imgui PUBLIC Vulkan::Vulkan sdl2)
endif()

set_target_third_party(imgui)

# Fetch tracy
FetchContent_Declare(
  tracy
  GIT_REPOSITORY https://github.com/wolfpld/tracy.git
  GIT_TAG        master
)
FetchContent_GetProperties(tracy)
if(NOT tracy_POPULATED)
  FetchContent_Populate(tracy)

  option(TRACY_ENABLE "" ON)
  option(TRACY_ON_DEMAND "" ON)
  option(TRACY_ONLY_LOCALHOST "" ON)
  add_subdirectory(${tracy_SOURCE_DIR} ${tracy_BINARY_DIR})

  set_target_third_party(TracyClient)
endif()

set_target_third_party(imgui)

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

add_subdirectory("${ENGINE_DIR}")

if (DEFINED APPLICATION_NAME)
  add_subdirectory("${APPLICATION_NAME}")
endif()