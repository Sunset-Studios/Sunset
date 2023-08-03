#include <image_serializer.h>
#include <maths.h>

#include <fstream>

#include <lz4.h>
#include <glm/glm.hpp>

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
		image_info.uncompressed_block_sizes = image_metadata["uncompressed_block_sizes"].get<std::vector<size_t>>();
		image_info.mip_buffer_start_indices = image_metadata["mip_buffer_sizes"].get<std::vector<size_t>>();
		image_info.mips = image_metadata["mips"];
		image_info.channels = image_metadata["channels"];

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
				LZ4_decompress_safe(source_buffer + compressed_block_offset, destination_buffer + block_idx * serialized_image_info->uncompressed_block_sizes[block_idx], compressed_block_size, PACKED_BUFFER_BLOCK_SIZE);
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
			int32_t uncompressed_block_offset = 0;
			for (uint32_t i = 0; i < serialized_image_info->compressed_block_sizes.size() && i < block_index; ++i)
			{
				compressed_block_offset += serialized_image_info->compressed_block_sizes[i];
				uncompressed_block_offset += serialized_image_info->uncompressed_block_sizes[i];
			}
			LZ4_decompress_safe(
				source_buffer + compressed_block_offset,
				destination_buffer + uncompressed_block_offset,
				serialized_image_info->compressed_block_sizes[block_index],
				PACKED_BUFFER_BLOCK_SIZE
			);
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
		SerializedAsset asset = pack_image_begin(serialized_image_info, image_metadata);
		const size_t total_compressed_buffer_size = pack_image_mip(serialized_image_info, asset, pixel_buffer, image_metadata, 0, serialized_image_info->extent[0], serialized_image_info->extent[1]);
		pack_image_end(asset, image_metadata, total_compressed_buffer_size);
		return asset;
	}

	SerializedAsset pack_image_begin(SerializedImageInfo* serialized_image_info, nlohmann::json& out_metadata)
	{
		out_metadata["format"] = SUNSET_FORMAT_TO_STRING(serialized_image_info->format);
		out_metadata["width"] = serialized_image_info->extent[0];
		out_metadata["height"] = serialized_image_info->extent[1];
		out_metadata["depth"] = serialized_image_info->extent[2];
		out_metadata["file_path"] = serialized_image_info->file_path;
		out_metadata["mips"] = serialized_image_info->mips;
		out_metadata["channels"] = serialized_image_info->channels;

		SerializedAsset asset;
		asset.type[0] = 'I';
		asset.type[1] = 'M';
		asset.type[2] = 'A';
		asset.type[3] = 'G';

		asset.version = 1;

		const uint32_t pot_buffer_size = Maths::ppot(serialized_image_info->size);
		size_t asset_buffer_size = serialized_image_info->size;
		for (uint32_t m = 1; m < serialized_image_info->mips; ++m)
		{
			asset_buffer_size += pot_buffer_size >> (m * 2);
		}
		out_metadata["buffer_size"] = asset_buffer_size;

		// Overestimate amount of allocated buffer space so that total_compressed_buffer_size that are slightly larger than our original image size
		// Don't cause a delete on unsanctioned memory when we try to resize later on
		asset.binary.resize(asset_buffer_size + asset_buffer_size * 0.5f);

		std::vector<size_t> compressed_block_sizes;
		compressed_block_sizes.reserve((asset_buffer_size / PACKED_BUFFER_BLOCK_SIZE) + 1);
		out_metadata["compressed_block_sizes"] = compressed_block_sizes;

		std::vector<size_t> uncompressed_block_sizes;
		uncompressed_block_sizes.reserve(compressed_block_sizes.capacity());
		out_metadata["uncompressed_block_sizes"] = uncompressed_block_sizes;

		std::vector<size_t> mip_buffer_start_indices;
		mip_buffer_start_indices.resize(serialized_image_info->mips, 0);
		out_metadata["mip_buffer_sizes"] = mip_buffer_start_indices;

		return asset;
	}

	size_t pack_image_mip(SerializedImageInfo* serialized_image_info, SerializedAsset& asset, void* pixel_buffer, nlohmann::json& metadata, uint32_t mip, size_t mip_width, size_t mip_height, size_t dst_buffer_offset)
	{
		std::vector<size_t> block_sizes = metadata["compressed_block_sizes"].get<std::vector<size_t>>();
		std::vector<size_t> uncompressed_block_sizes = metadata["uncompressed_block_sizes"].get<std::vector<size_t>>();
		std::vector<size_t> mip_buffer_sizes = metadata["mip_buffer_sizes"].get<std::vector<size_t>>();

		const size_t mip_size = mip_width * mip_height * serialized_image_info->channels;

		size_t total_compressed_buffer_size{ 0 };
		size_t total_uncompressed_buffer_size{ 0 };
		for (size_t i = 0; i < mip_size; i += PACKED_BUFFER_BLOCK_SIZE)
		{
			const int current_block_size = std::min(PACKED_BUFFER_BLOCK_SIZE, mip_size - i);
			const int compressed_staging_size = LZ4_compressBound(current_block_size);

			const size_t compressed_block_size = LZ4_compress_default((const char*)pixel_buffer + i, asset.binary.data() + dst_buffer_offset + total_compressed_buffer_size, current_block_size, compressed_staging_size);
			block_sizes.push_back(compressed_block_size);
			uncompressed_block_sizes.push_back(current_block_size);

			total_compressed_buffer_size += compressed_block_size;
			total_uncompressed_buffer_size += current_block_size;
		}
		mip_buffer_sizes[mip] = total_uncompressed_buffer_size;

		metadata["compressed_block_sizes"] = block_sizes;
		metadata["uncompressed_block_sizes"] = uncompressed_block_sizes;
		metadata["mip_buffer_sizes"] = mip_buffer_sizes;

		return total_compressed_buffer_size;
	}

	void pack_image_end(SerializedAsset& asset, nlohmann::json& metadata, size_t total_compressed_size)
	{
		asset.binary.resize(total_compressed_size);

		metadata["compression"] = SUNSET_COMPRESSION_MODE_TO_STRING(CompressionMode::LZ4);

		std::string stringified_metadata = metadata.dump();
		asset.metadata = stringified_metadata;
	}
}
