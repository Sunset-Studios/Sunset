#pragma once

#include <serializer.h>

#include <string>
#include <vector>

namespace Sunset
{
	struct SerializedImageInfo
	{
		uint64_t size;
		Format format;
		CompressionMode compression_mode;
		uint32_t extent[3];
		std::string file_path;
		std::vector<size_t> compressed_block_sizes;
	};

	SerializedImageInfo get_serialized_image_info(struct SerializedAsset* asset);
	void unpack_image(SerializedImageInfo* serialized_image_info, const char* source_buffer, size_t source_buffer_size, char* destination_buffer);
	void unpack_image_block(SerializedImageInfo* serialized_image_info, const char* source_buffer, char* destination_buffer, size_t block_index);
	SerializedAsset pack_image(SerializedImageInfo* serialized_image_info, void* pixel_buffer);
}
