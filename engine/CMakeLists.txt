MACRO(HEADER_DIRECTORIES return_list base_path)
    FILE(GLOB_RECURSE new_list ${base_path}/*.h)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

macro(create_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if(IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _group_path "${_source_path}")
        source_group("${_group_path}" FILES "${_source}")
    endforeach()
endmacro()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.cpp" "shaders/*.vert" "shaders/*.frag" "shaders/*.inc" "shaders/*.comp")

HEADER_DIRECTORIES(INCLUDE_DIRS src)

list(APPEND INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/third_party")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tools/libs")

add_library(Sunset STATIC ${SOURCES})

set_property(TARGET Sunset PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Sunset>")

target_compile_definitions(Sunset PRIVATE PROJECT_PATH="${CMAKE_SOURCE_DIR}")

target_link_libraries(Sunset vkbootstrap vma glm imgui lz4 json spirv_reflect Vulkan::Vulkan sdl2 AssetSerializer Tracy::TracyClient Jolt atomic_queue parallel_hashmap)

target_include_directories(Sunset PUBLIC ${INCLUDE_DIRS} ${JoltPhysics_SOURCE_DIR}/../Jolt)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tools/apps")

add_custom_command(TARGET ${SUNSET_LIB} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdl2/lib/x64/SDL2.dll" ${ENGINE_DIR}/bin/$<CONFIG>/SDL2.dll
                  )

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests")

create_source_group(${SOURCES})