#include <shader_serializer.h>

#include <fstream>

#include <json.hpp>
#include <lz4.h>

namespace Sunset
{
	Sunset::SerializedShaderInfo get_serialized_shader_info(SerializedAsset* asset)
	{
		SerializedShaderInfo shader_info;

		nlohmann::json shader_metadata = nlohmann::json::parse(asset->metadata);

		std::string compression_mode = shader_metadata["compression"];
		shader_info.compression_mode = SUNSET_COMPRESSION_MODE_FROM_STRING(compression_mode.c_str());

		shader_info.shader_buffer_size = shader_metadata["buffer_size"];
		shader_info.file_path = shader_metadata["file_path"];

		return shader_info;
	}

	void unpack_shader(SerializedShaderInfo* serialized_shader_info, const char* source_buffer, size_t source_buffer_size, char* destination_shader_buffer)
	{
		if (serialized_shader_info->compression_mode == CompressionMode::LZ4)
		{
			LZ4_decompress_safe(source_buffer, destination_shader_buffer, source_buffer_size, serialized_shader_info->shader_buffer_size);
		}
		else
		{
			memcpy(destination_shader_buffer, source_buffer, source_buffer_size);
		}
	}

	Sunset::SerializedAsset pack_shader(SerializedShaderInfo* serialized_shader_info, void* shader_data)
	{
		nlohmann::json shader_metadata;
		shader_metadata["buffer_size"] = serialized_shader_info->shader_buffer_size;
		shader_metadata["file_path"] = serialized_shader_info->file_path;

		SerializedAsset asset;
		asset.type[0] = 'S';
		asset.type[1] = 'H';
		asset.type[2] = 'A';
		asset.type[3] = 'D';

		asset.version = 1;

		int compressed_staging_size = LZ4_compressBound(serialized_shader_info->shader_buffer_size);

		asset.binary.resize(compressed_staging_size);

		int compressed_buffer_size = LZ4_compress_default((const char*)shader_data, asset.binary.data(), serialized_shader_info->shader_buffer_size, compressed_staging_size);

		asset.binary.resize(compressed_buffer_size);

		shader_metadata["compression"] = SUNSET_COMPRESSION_MODE_TO_STRING(CompressionMode::LZ4);

		std::string stringified_metadata = shader_metadata.dump();
		asset.metadata = stringified_metadata;

		return asset;
	}
}
