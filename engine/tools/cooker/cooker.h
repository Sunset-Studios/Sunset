#pragma once

#include <filesystem>

namespace Sunset
{
	class Cooker
	{
	public:
		static bool cook_image(const std::filesystem::path& input_path, const std::filesystem::path& output_path);
		static bool cook_mesh(const std::filesystem::path& input_path, const std::filesystem::path& output_path);
		static bool cook_shader(const std::filesystem::path& input_path, const std::filesystem::path& output_path);
	};
}
