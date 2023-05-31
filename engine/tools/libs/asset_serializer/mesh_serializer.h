#pragma once

#include <serializer.h>

#include <string>
#include <vector>

namespace Sunset
{
	enum class VertexFormat : uint32_t
	{
		Undefined = 0,
		PNCU32,
		PNCUTB32,
		P32N8C8U16
	};

	struct VertexPNCU32
	{
		float position[3];
		float normal[3];
		float color[3];
		float uv[2];
	};

	struct VertexPNCUTB32
	{
		float position[3];
		float normal[3];
		float color[3];
		float uv[2];
		float tangent[3];
		float bitangent[3];
	};

	struct VertexP32N8C8U16
	{
		float position[3];
		uint16_t normal_color[3];
		float uv[2];
	};

	struct MeshBounds
	{
		float origin[3];
		float extents[3];
		float radius;
	};

	struct SerializedMeshInfo
	{
		uint64_t vertex_buffer_size;
		uint64_t index_buffer_size;
		MeshBounds bounds;
		VertexFormat format;
		CompressionMode compression_mode;
		char index_size;
		std::string file_path;
	};

	SerializedMeshInfo get_serialized_mesh_info(struct SerializedAsset* asset);
	void unpack_mesh(SerializedMeshInfo* serialized_image_info, const char* source_buffer, size_t source_buffer_size, char* destination_vertex_buffer, char* destination_index_buffer);
	SerializedAsset pack_mesh(SerializedMeshInfo* serialized_image_info, void* vertex_data, void* index_data);
	MeshBounds calculate_mesh_bounds(VertexPNCUTB32* vertices, size_t vertex_count);

	inline VertexFormat SUNSET_VERTEX_FORMAT_FROM_STRING(const char* format_str)
	{
		if (strcmp(format_str, "PNCU32") == 0)
		{
			return VertexFormat::PNCU32;
		}
		else if (strcmp(format_str, "P32N8C8U16") == 0)
		{
			return VertexFormat::P32N8C8U16;
		}
		else if (strcmp(format_str, "PNCUTB32") == 0)
		{
			return VertexFormat::PNCUTB32;
		}
		else
		{
			return VertexFormat::Undefined;
		}
	}

	inline const char* SUNSET_VERTEX_FORMAT_TO_STRING(VertexFormat format)
	{
		switch (format)
		{
		case VertexFormat::PNCU32:
			return "PNCU32";
		case VertexFormat::P32N8C8U16:
			return "P32N8C8U16";
		case VertexFormat::PNCUTB32:
			return "PNCUTB32";
		default:
			return "Undefined";
		}
	}
}
