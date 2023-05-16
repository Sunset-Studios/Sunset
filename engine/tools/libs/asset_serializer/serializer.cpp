#include <serializer.h>

#include <fstream>

namespace Sunset
{
	bool serialize_asset(const char* path, const SerializedAsset& asset)
	{
		std::ofstream out_file;
		out_file.open(path, std::ios::binary | std::ios::out);

		out_file.write(asset.type, 4);

		uint32_t version = asset.version;
		out_file.write((const char*)&version, sizeof(uint32_t));

		uint32_t metadata_length = asset.metadata.size();
		out_file.write((const char*)&metadata_length, sizeof(uint32_t));

		uint32_t binary_length = asset.binary.size();
		out_file.write((const char*)&binary_length, sizeof(uint32_t));

		out_file.write(asset.metadata.data(), metadata_length);
		out_file.write(asset.binary.data(), asset.binary.size());

		out_file.close();

		return true;
	}

	bool deserialize_asset(const char* path, SerializedAsset& out_asset)
	{
		std::ifstream in_file;
		in_file.open(path, std::ios::binary);

		if (!in_file.is_open())
		{
			return false;
		}

		in_file.seekg(0);

		in_file.read(out_asset.type, 4);
		in_file.read((char*)&out_asset.version, sizeof(uint32_t));

		uint32_t metadata_length{ 0 };
		in_file.read((char*)&metadata_length, sizeof(uint32_t));

		uint32_t binary_length{ 0 };
		in_file.read((char*)&binary_length, sizeof(uint32_t));

		out_asset.metadata.resize(metadata_length);
		in_file.read(out_asset.metadata.data(), metadata_length);

		out_asset.binary.resize(binary_length);
		in_file.read(out_asset.binary.data(), binary_length);

		return true;
	}
}
