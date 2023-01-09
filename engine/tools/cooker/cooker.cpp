#include <cooker.h>
#include <image_serializer.h>

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Sunset
{
	bool Cooker::cook_image(const std::filesystem::path& input_path, const std::filesystem::path& output_path)
	{
		int texture_width, texture_height, texture_channels;

		stbi_uc* pixel_buffer = stbi_load((const char*)input_path.u8string().c_str(), &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);

		if (pixel_buffer == nullptr)
		{
			std::cout << "Failed to load image file " << input_path << std::endl;
		}

		int texture_size = texture_width * texture_height * 4;

		SerializedImageInfo image_info;
		image_info.size = texture_size;
		image_info.extent[0] = texture_width;
		image_info.extent[1] = texture_height;
		image_info.extent[2] = 1;
		image_info.format = Format::UNorm4x8;
		image_info.file_path = input_path.string();

		SerializedAsset new_image_asset = pack_image(&image_info, pixel_buffer);

		stbi_image_free(pixel_buffer);

		serialize_asset(output_path.string().c_str(), new_image_asset);

		return true;
	}

	bool Cooker::cook_mesh(const std::filesystem::path& input_path, const std::filesystem::path& output_path)
	{
		return true;
	}
}
