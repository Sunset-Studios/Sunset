#pragma once

#include <serializer.h>

#include <string>
#include <vector>
#include <fstream>

namespace Sunset
{
	struct SerializedShaderInfo
	{
		uint64_t shader_buffer_size;
		CompressionMode compression_mode;
		std::string file_path;
	};

	SerializedShaderInfo get_serialized_shader_info(struct SerializedAsset* asset);
	void unpack_shader(SerializedShaderInfo* serialized_shader_info, const char* source_buffer, size_t source_buffer_size, char* destination_shader_buffer);
	SerializedAsset pack_shader(SerializedShaderInfo* serialized_shader_info, void* shader_data);
	void parse_shader_includes(std::string& shader_code);
	std::string read_shader_file(const std::filesystem::path& input_path);
}
