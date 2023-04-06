MACRO(HEADER_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list src/*.h third_party/*.h third_party/*.hpp)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.cpp")

HEADER_DIRECTORIES(INCLUDE_DIRS)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/third_party")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tools")

add_executable(Sunset ${SOURCES})

set_property(TARGET Sunset PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Sunset>")

target_include_directories(Sunset PUBLIC ${INCLUDE_DIRS})

target_link_libraries(Sunset vkbootstrap vma glm imgui lz4 json spirv_reflect)
target_link_libraries(Sunset Vulkan::Vulkan sdl2 AssetSerializer Tracy::TracyClient)

add_custom_command(TARGET ${SUNSET_LIB} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy "C:/SDL2/lib/x64/SDL2.dll" ${ENGINE_DIR}/bin/$<CONFIG>/SDL2.dll
                  )