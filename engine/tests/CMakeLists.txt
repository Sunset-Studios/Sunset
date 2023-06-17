enable_testing()

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(SunsetTests ${TEST_SOURCES})

target_include_directories(SunsetTests PUBLIC ${INCLUDE_DIRS})

target_link_libraries(SunsetTests Sunset benchmark::benchmark GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(SunsetTests)