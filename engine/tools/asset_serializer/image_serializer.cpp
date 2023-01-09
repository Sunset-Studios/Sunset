#include <image_serializer.h>

#include <fstream>

#include <json.hpp>
#include <lz4.h>

namespace Sunset
{
	Sunset::SerializedImageInfo get_serialized_image_info(SerializedAsset* asset)
	{
		SerializedImageInfo image_info;

		nlohmann::json image_metadata = nlohmann::json::parse(asset->metadata);

		std::string format = image_metadata["format"];
		image_info.format = SUNSET_FORMAT_FROM_STRING(format.c_str());

		std::string compression_mode = image_metadata["compression"];
		image_info.compression_mode = SUNSET_COMPRESSION_MODE_FROM_STRING(compression_mode.c_str());

		image_info.extent[0] = image_metadata["width"];
		image_info.extent[1] = image_metadata["height"];
		image_info.extent[2] = image_metadata["depth"];
		image_info.size = image_metadata["buffer_size"];
		image_info.file_path = image_metadata["file_path"];

		return image_info;
	}

	void unpack_image(SerializedImageInfo* serialized_image_info, const char* source_buffer, size_t source_buffer_size, char* destination_buffer)
	{
		if (serialized_image_info->compression_mode == CompressionMode::LZ4)
		{
			LZ4_decompress_safe(source_buffer, destination_buffer, source_buffer_size, serialized_image_info->size);
		}
		else
		{
			memcpy(destination_buffer, source_buffer, source_buffer_size);
		}
	}

	Sunset::SerializedAsset pack_image(SerializedImageInfo* serialized_image_info, void* pixel_buffer)
	{
		nlohmann::json image_metadata;
		image_metadata["format"] = SUNSET_FORMAT_TO_STRING(serialized_image_info->format);
		image_metadata["width"] = serialized_image_info->extent[0];
		image_metadata["height"] = serialized_image_info->extent[1];
		image_metadata["depth"] = serialized_image_info->extent[2];
		image_metadata["buffer_size"] = serialized_image_info->size;
		image_metadata["file_path"] = serialized_image_info->file_path;

		SerializedAsset asset;
		asset.type[0] = 'I';
		asset.type[1] = 'M';
		asset.type[2] = 'A';
		asset.type[3] = 'G';

		asset.version = 1;

		int compressed_staging_size = LZ4_compressBound(serialized_image_info->size);

		asset.binary.resize(compressed_staging_size);

		int compressed_buffer_size = LZ4_compress_default((const char*)pixel_buffer, asset.binary.data(), serialized_image_info->size, compressed_staging_size);

		asset.binary.resize(compressed_buffer_size);

		image_metadata["compression"] = SUNSET_COMPRESSION_MODE_TO_STRING(CompressionMode::LZ4);

		std::string stringified_metadata = image_metadata.dump();
		asset.metadata = stringified_metadata;

		return asset;
	}
}
