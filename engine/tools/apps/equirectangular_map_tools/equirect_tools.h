// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>
#include <glm/glm.hpp>

#include <core/common.h>

#include <filesystem>

namespace Sunset
{
	class EquirectToolsApplication
	{
		public:
			void init(const std::filesystem::path& equirect_path, bool generate_cubemap = false, bool generate_irradiance_map = false);
			void cleanup();
			void run();

		private:
			std::filesystem::path create_save_directory(const std::filesystem::path& equirect_path);
			void load_equirect_image(const std::filesystem::path& equirect_path);

			void create_equirect_cubemap();
			void create_irradiance_map();

			void write_cubemap_to_png(const std::filesystem::path& out_path);
			void write_irradiance_to_png(const std::filesystem::path& out_path);

		public:
			static ImageID equirect_image;
			static ImageID equirect_cubemap_image;
			static ImageID irradiance_map_image;

		private:
			std::filesystem::path parent_equirect_path;
			std::string equirect_name;
			bool b_generate_cubemap_textures{ false };
			bool b_generate_irradiance_map{ false };
	};
}
