list(APPEND INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer")

add_library(AssetSerializer STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer/serializer.h
	${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer/serializer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer/image_serializer.h
	${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer/image_serializer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer/mesh_serializer.h
	${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer/mesh_serializer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer/shader_serializer.h
	${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer/shader_serializer.cpp

)

set_property(TARGET AssetSerializer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:AssetSerializer>")

target_compile_definitions(AssetSerializer PRIVATE PROJECT_PATH="${CMAKE_SOURCE_DIR}")

target_include_directories(AssetSerializer PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/asset_serializer
	${CMAKE_SOURCE_DIR}/engine/src/core
	${CMAKE_SOURCE_DIR}/engine/src/utility
	${CMAKE_SOURCE_DIR}/engine/third_party/tracy/public
	${CMAKE_SOURCE_DIR}/engine/third_party/parallel_hashmap)

target_link_libraries(AssetSerializer PRIVATE json lz4 glm parallel_hashmap)
