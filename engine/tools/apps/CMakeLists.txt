set(COOKER_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/cooker/cooker.h
	${CMAKE_CURRENT_SOURCE_DIR}/cooker/cooker.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cooker/main.cpp)

add_executable(SunsetCooker ${COOKER_SOURCES})

set_property(TARGET SunsetCooker PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:SunsetCooker>")

target_compile_definitions(SunsetCooker PRIVATE PROJECT_PATH="${CMAKE_SOURCE_DIR}")

target_include_directories(SunsetCooker PUBLIC
	${shaderc_SOURCE_DIR}/libshaderc/include
	${CMAKE_CURRENT_SOURCE_DIR}/cooker
	${CMAKE_CURRENT_SOURCE_DIR}/../libs/asset_serializer)

target_link_libraries(SunsetCooker PRIVATE tinyobjloader stb_image json lz4 glm AssetSerializer shaderc open_fbx)

set(EQUIRECT_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/equirectangular_map_tools/equirect_tools.h
	${CMAKE_CURRENT_SOURCE_DIR}/equirectangular_map_tools/equirect_tools.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/equirectangular_map_tools/main.cpp)

add_executable(EquirectTools ${EQUIRECT_SOURCES})

set_property(TARGET EquirectTools PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:EquirectTools>")

target_compile_definitions(EquirectTools PRIVATE PROJECT_PATH="${CMAKE_SOURCE_DIR}")

target_include_directories(EquirectTools PUBLIC
	"${INCLUDE_DIRS}"	
	"${CMAKE_CURRENT_SOURCE_DIR}/equirectangular_map_tools"
)

target_link_libraries(EquirectTools PRIVATE Sunset stb_image)

add_custom_target(CookerRun
	COMMAND SunsetCooker "${CMAKE_SOURCE_DIR}/engine/assets" "${CMAKE_SOURCE_DIR}/engine/shaders"
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	COMMENT "Cooking assets in ${CMAKE_CURRENT_SOURCE_DIR}"
	SOURCES ${COOKER_SOURCES})
