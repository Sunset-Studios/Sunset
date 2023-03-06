#include <mesh_serializer.h>

#include <fstream>

#include <json.hpp>
#include <lz4.h>

namespace Sunset
{
	Sunset::SerializedMeshInfo get_serialized_mesh_info(SerializedAsset* asset)
	{
		SerializedMeshInfo mesh_info;

		nlohmann::json metadata = nlohmann::json::parse(asset->metadata);

		mesh_info.vertex_buffer_size = metadata["vertex_buffer_size"];
		mesh_info.index_buffer_size = metadata["index_buffer_size"];
		mesh_info.index_size = (uint8_t) metadata["index_size"];
		mesh_info.file_path = metadata["file_path"];

		std::string compression_mode_str = metadata["compression"];
		mesh_info.compression_mode = SUNSET_COMPRESSION_MODE_FROM_STRING(compression_mode_str.c_str());

		std::vector<float> bounds_data;
		bounds_data.reserve(7);
		bounds_data = metadata["bounds"].get<std::vector<float>>();

		mesh_info.bounds.origin[0] = bounds_data[0];
		mesh_info.bounds.origin[1] = bounds_data[1];
		mesh_info.bounds.origin[2] = bounds_data[2];

		mesh_info.bounds.extents[0] = bounds_data[3];
		mesh_info.bounds.extents[1] = bounds_data[4];
		mesh_info.bounds.extents[2] = bounds_data[5];

		mesh_info.bounds.radius = bounds_data[6];

		std::string vertex_format_str = metadata["vertex_format"];
		mesh_info.format = SUNSET_VERTEX_FORMAT_FROM_STRING(vertex_format_str.c_str());

		return mesh_info;
	}

	void unpack_mesh(SerializedMeshInfo* serialized_mesh_info, const char* source_buffer, size_t source_buffer_size, char* destination_vertex_buffer, char* destination_index_buffer)
	{
		std::vector<char> decompressed_buffer;
		decompressed_buffer.resize(serialized_mesh_info->vertex_buffer_size + serialized_mesh_info->index_buffer_size);

		if (serialized_mesh_info->compression_mode == CompressionMode::LZ4)
		{
			LZ4_decompress_safe(source_buffer, decompressed_buffer.data(), static_cast<int>(source_buffer_size), static_cast<int>(decompressed_buffer.size()));
		}
		else
		{
			memcpy(decompressed_buffer.data(), source_buffer, source_buffer_size);
		}

		memcpy(destination_vertex_buffer, decompressed_buffer.data(), serialized_mesh_info->vertex_buffer_size);
		memcpy(destination_index_buffer, decompressed_buffer.data() + serialized_mesh_info->vertex_buffer_size, serialized_mesh_info->index_buffer_size);
	}

	Sunset::SerializedAsset pack_mesh(SerializedMeshInfo* serialized_mesh_info, void* vertex_data, void* index_data)
	{
		nlohmann::json mesh_metadata;
		mesh_metadata["vertex_format"] = SUNSET_VERTEX_FORMAT_TO_STRING(serialized_mesh_info->format);
		mesh_metadata["vertex_buffer_size"] = serialized_mesh_info->vertex_buffer_size;
		mesh_metadata["index_buffer_size"] = serialized_mesh_info->index_buffer_size;
		mesh_metadata["index_size"] = serialized_mesh_info->index_size;
		mesh_metadata["file_path"] = serialized_mesh_info->file_path;

		SerializedAsset asset;
		asset.type[0] = 'M';
		asset.type[1] = 'E';
		asset.type[2] = 'S';
		asset.type[3] = 'H';

		asset.version = 1;

		std::vector<float> bounds_data;
		bounds_data.resize(7);

		bounds_data[0] = serialized_mesh_info->bounds.origin[0];
		bounds_data[1] = serialized_mesh_info->bounds.origin[1];
		bounds_data[2] = serialized_mesh_info->bounds.origin[2];

		bounds_data[3] = serialized_mesh_info->bounds.extents[0];
		bounds_data[4] = serialized_mesh_info->bounds.extents[1];
		bounds_data[5] = serialized_mesh_info->bounds.extents[2];

		bounds_data[6] = serialized_mesh_info->bounds.radius;

		mesh_metadata["bounds"] = bounds_data;

		size_t full_size = serialized_mesh_info->vertex_buffer_size + serialized_mesh_info->index_buffer_size;

		std::vector<char> merged_buffer;
		merged_buffer.resize(full_size);

		memcpy(merged_buffer.data(), vertex_data, serialized_mesh_info->vertex_buffer_size);
		memcpy(merged_buffer.data() + serialized_mesh_info->vertex_buffer_size, index_data, serialized_mesh_info->index_buffer_size);

		size_t compressed_staging_size = LZ4_compressBound(static_cast<int>(full_size));

		asset.binary.resize(compressed_staging_size);

		size_t compressed_size = LZ4_compress_default(merged_buffer.data(), asset.binary.data(), static_cast<int>(merged_buffer.size()), static_cast<int>(compressed_staging_size));

		asset.binary.resize(compressed_size);

		mesh_metadata["compression"] = SUNSET_COMPRESSION_MODE_TO_STRING(CompressionMode::LZ4);

		asset.metadata = mesh_metadata.dump();

		return asset;
	}

	Sunset::MeshBounds calculate_mesh_bounds(VertexPNCU32* vertices, size_t vertex_count)
	{
		MeshBounds bounds;

		const float max_float = std::numeric_limits<float>::max();
		const float min_float = std::numeric_limits<float>::min();
		float min[3] = { max_float, max_float, max_float };
		float max[3] = { min_float, min_float, min_float };

		for (int i = 0; i < vertex_count; ++i)
		{
			min[0] = std::min(min[0], vertices[i].position[0]);
			min[1] = std::min(min[1], vertices[i].position[1]);
			min[2] = std::min(min[2], vertices[i].position[2]);

			max[0] = std::max(max[0], vertices[i].position[0]);
			max[1] = std::max(max[1], vertices[i].position[1]);
			max[2] = std::max(max[2], vertices[i].position[2]);
		}

		bounds.extents[0] = (max[0] - min[0]) / 2.0f;
		bounds.extents[1] = (max[1] - min[1]) / 2.0f;
		bounds.extents[2] = (max[2] - min[2]) / 2.0f;

		bounds.origin[0] = bounds.extents[0] + min[0];
		bounds.origin[1] = bounds.extents[1] + min[1];
		bounds.origin[2] = bounds.extents[2] + min[2];

		float radius_squared = 0;
		for (int i = 0; i < vertex_count; ++i)
		{
			float offset[3];

			offset[0] = vertices[i].position[0] - bounds.origin[0];
			offset[1] = vertices[i].position[1] - bounds.origin[1];
			offset[2] = vertices[i].position[2] - bounds.origin[2];

			float distance = offset[0] * offset[0] + offset[1] * offset[1] + offset[2] * offset[2];
			radius_squared = std::max(radius_squared, distance);
		}

		bounds.radius = std::sqrt(radius_squared);

		return bounds;
	}
}
