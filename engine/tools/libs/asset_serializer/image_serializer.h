#pragma once

#include <serializer.h>

#include <json.hpp>

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
		std::vector<size_t> mip_buffer_start_indices;
		uint32_t mips;
		uint32_t channels;
	};

	SerializedImageInfo get_serialized_image_info(struct SerializedAsset* asset);
	void unpack_image(SerializedImageInfo* serialized_image_info, const char* source_buffer, size_t source_buffer_size, char* destination_buffer);
	void unpack_image_block(SerializedImageInfo* serialized_image_info, const char* source_buffer, char* destination_buffer, size_t block_index);

	SerializedAsset pack_image(SerializedImageInfo* serialized_image_info, void* pixel_buffer);
	SerializedAsset pack_image_begin(SerializedImageInfo* serialized_image_info, nlohmann::json& out_metadata);
	size_t pack_image_mip(SerializedImageInfo* serialized_image_info, SerializedAsset& asset, void* pixel_buffer, nlohmann::json& metadata, uint32_t mip, size_t mip_width, size_t mip_height, size_t dst_buffer_offset = 0);
	void pack_image_end(SerializedAsset& asset, nlohmann::json& metadata, size_t total_compressed_size);
}
