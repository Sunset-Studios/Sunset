list(APPEND INCLUDE_DIRS
	"${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer")

add_library(AssetSerializer STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer/serializer.h
	${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer/serializer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer/image_serializer.h
	${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer/image_serializer.cpp
)

set_property(TARGET AssetSerializer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:AssetSerializer>")

target_include_directories(AssetSerializer PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer"
	"${CMAKE_SOURCE_DIR}/engine/src/core")

target_link_libraries(AssetSerializer PRIVATE json lz4 glm)

set(COOKER_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/cooker/cooker.h
	${CMAKE_CURRENT_SOURCE_DIR}/cooker/cooker.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cooker/main.cpp)

add_executable(SunsetCooker ${COOKER_SOURCES})

set_property(TARGET SunsetCooker PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:SunsetCooker>")

target_include_directories(SunsetCooker PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/cooker"
	"${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer")

target_link_libraries(SunsetCooker PRIVATE tinyobjloader stb_image json lz4 glm AssetSerializer)

add_custom_target(CookerRun
	COMMAND SunsetCooker "${CMAKE_SOURCE_DIR}/engine/assets"
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	COMMENT "Cooking assets in ${CMAKE_CURRENT_SOURCE_DIR}"
	SOURCES ${COOKER_SOURCES})