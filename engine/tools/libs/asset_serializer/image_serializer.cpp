#include <image_serializer.h>

#include <fstream>

#include <json.hpp>
#include <lz4.h>

namespace Sunset
{
	constexpr size_t PACKED_BUFFER_BLOCK_SIZE  = 64 * 1024; // 64KB blocks

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
		image_info.compressed_block_sizes = image_metadata["compressed_block_sizes"].get<std::vector<size_t>>();

		return image_info;
	}

	void unpack_image(SerializedImageInfo* serialized_image_info, const char* source_buffer, size_t source_buffer_size, char* destination_buffer)
	{
		if (serialized_image_info->compression_mode == CompressionMode::LZ4)
		{
			ZoneScopedN("unpack_image: LZ4_decompress_safe");
			int32_t compressed_block_offset = 0, block_idx = 0;
			for (size_t compressed_block_size : serialized_image_info->compressed_block_sizes)
			{
				LZ4_decompress_safe(source_buffer + compressed_block_offset, destination_buffer + block_idx * PACKED_BUFFER_BLOCK_SIZE, compressed_block_size, PACKED_BUFFER_BLOCK_SIZE);
				compressed_block_offset += compressed_block_size;
				++block_idx;
			}
		}
		else
		{
			ZoneScopedN("unpack_image: memcpy");
			memcpy(destination_buffer, source_buffer, source_buffer_size);
		}
	}

	void unpack_image_block(SerializedImageInfo* serialized_image_info, const char* source_buffer, char* destination_buffer, size_t block_index)
	{
		if (serialized_image_info->compression_mode == CompressionMode::LZ4)
		{
			ZoneScopedN("unpack_image_block: LZ4_decompress_safe");
			int32_t compressed_block_offset = 0;
			for (uint32_t i = 0; i < serialized_image_info->compressed_block_sizes.size() && i < block_index; ++i)
			{
				compressed_block_offset += serialized_image_info->compressed_block_sizes[i];
			}
			const size_t compressed_block_size = serialized_image_info->compressed_block_sizes[block_index];
			LZ4_decompress_safe(source_buffer + compressed_block_offset, destination_buffer + block_index * PACKED_BUFFER_BLOCK_SIZE, compressed_block_size, PACKED_BUFFER_BLOCK_SIZE);
		}
		else
		{
			ZoneScopedN("unpack_image_block: memcpy");
			memcpy(destination_buffer + block_index * PACKED_BUFFER_BLOCK_SIZE, source_buffer + block_index * PACKED_BUFFER_BLOCK_SIZE, PACKED_BUFFER_BLOCK_SIZE);
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

		// Overestimate amount of allocated buffer space so that total_compressed_buffer_size that are slightly larger than our original image size
		// Don't cause a delete on unsanctioned memory when we try to resize later on
		asset.binary.resize(serialized_image_info->size + serialized_image_info->size * 0.5f);

		std::vector<size_t> compressed_block_sizes;
		compressed_block_sizes.reserve((serialized_image_info->size / PACKED_BUFFER_BLOCK_SIZE) + 1);

		int total_compressed_buffer_size{ 0 };
		for (size_t i = 0; i < serialized_image_info->size; i += PACKED_BUFFER_BLOCK_SIZE) 
		{
			const int current_block_size = std::min(PACKED_BUFFER_BLOCK_SIZE, serialized_image_info->size - i);
			const int compressed_staging_size = LZ4_compressBound(current_block_size);

			const size_t compressed_block_size = LZ4_compress_default((const char*)pixel_buffer + i, asset.binary.data() + total_compressed_buffer_size, current_block_size, compressed_staging_size);
			compressed_block_sizes.push_back(compressed_block_size);

			total_compressed_buffer_size += compressed_block_size;
		}

		asset.binary.resize(total_compressed_buffer_size);

		image_metadata["compression"] = SUNSET_COMPRESSION_MODE_TO_STRING(CompressionMode::LZ4);
		image_metadata["compressed_block_sizes"] = compressed_block_sizes;

		std::string stringified_metadata = image_metadata.dump();
		asset.metadata = stringified_metadata;

		return asset;
	}
}
