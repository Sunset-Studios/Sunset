#include <shader_serializer.h>

#include <regex>
#include <format>

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

		asset.binary.resize(serialized_shader_info->shader_buffer_size);
		memcpy(asset.binary.data(), shader_data, serialized_shader_info->shader_buffer_size);

		shader_metadata["compression"] = SUNSET_COMPRESSION_MODE_TO_STRING(CompressionMode::None);

		std::string stringified_metadata = shader_metadata.dump();
		asset.metadata = stringified_metadata;

		return asset;
	}

	void parse_shader_includes(std::string& shader_code)
	{
		std::vector<size_t> include_positions;

		size_t pos = shader_code.find("#include", 0);
		while (pos != std::string::npos)
		{
			include_positions.push_back(pos);
			pos = shader_code.find("#include", pos + 1);
		}

		std::regex include_regex("^#include\\s+\"(\\S+)\".*$");

		std::smatch match;
		for (auto it = include_positions.rbegin(); it != include_positions.rend(); it++)
		{
			const size_t start = *it;
			const size_t end = shader_code.find('\r\n', *it);
			std::string include_line = shader_code.substr(start, (end - start));
			if (std::regex_search(include_line, match, include_regex))
			{
				std::string include_path = std::format("{}/engine/shaders", PROJECT_PATH);
				include_path += (match[1].str().front() != '/' ? "/" : "") + match[1].str();
				std::string include_contents = read_shader_file(include_path);
				parse_shader_includes(include_contents);
				shader_code = shader_code.replace(start, (end - start), include_contents);
			}
		}
	}

	std::string read_shader_file(const std::filesystem::path& input_path)
	{
		std::ifstream file(input_path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			return std::string();
		}

		size_t file_size = static_cast<size_t>(file.tellg());

		std::vector<char> buffer(file_size + 1);

		file.seekg(0);

		file.read(buffer.data(), file_size);

		file.close();

		buffer.push_back('\0');

		std::string shader_string(buffer.data());

		return shader_string;
	}
}
