file(GLOB_RECURSE DEMO_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/demo/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/demo/*.cpp")

HEADER_DIRECTORIES(DEMO_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/demo")
list(APPEND DEMO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/demo)

add_executable(SunsetDemo ${DEMO_SOURCES})

set_property(TARGET SunsetDemo PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:SunsetDemo>")

target_include_directories(SunsetDemo PUBLIC ${INCLUDE_DIRS} ${DEMO_INCLUDE_DIRS})

target_link_libraries(SunsetDemo Sunset)