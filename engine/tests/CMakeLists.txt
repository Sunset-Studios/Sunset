file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(SunsetTests ${TEST_SOURCES})

target_include_directories(SunsetTests PUBLIC ${INCLUDE_DIRS})

target_link_libraries(SunsetTests benchmark::benchmark)
target_link_libraries(SunsetTests vkbootstrap vma glm tinyobjloader imgui stb_image)
target_link_libraries(SunsetTests Vulkan::Vulkan sdl2)
