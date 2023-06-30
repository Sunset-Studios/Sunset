#pragma once

#include <filesystem>

#ifndef PROJECT_PATH
#define PROJECT_PATH ""
#endif

namespace Sunset
{
	class Cooker
	{
	public:
		static bool cook_image(const std::filesystem::path& input_path, const std::filesystem::path& output_path);
		static bool cook_mesh_obj(const std::filesystem::path& input_path, const std::filesystem::path& output_path);
		static bool cook_mesh_fbx(const std::filesystem::path& input_path, const std::filesystem::path& output_path);
		static bool cook_shader(const std::filesystem::path& input_path, const std::filesystem::path& output_path);
	};
}
