#include <equirect_tools.h>

#include <iostream>
#include <filesystem>

int main(int argc, char* argv[])
{
	bool b_generate_cubemap_textures = false;
	bool b_generate_irradiance_map = false;
	std::filesystem::path equirect_map_path;

	for (uint32_t i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--generate_cubemap_textures") == 0)
		{
			b_generate_cubemap_textures = true;
		}
		else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--generate_irradiance_map") == 0)
		{
			b_generate_irradiance_map = true;
		}
		else
		{
			const std::filesystem::path file_path{ argv[i] };
			const std::filesystem::path extension = file_path.extension();

			if ((extension == ".png" || extension == ".exr" || extension == ".hdr" || extension == ".png") && std::filesystem::exists(file_path))
			{
				equirect_map_path = file_path;
			}
		}
	}

	if ((!b_generate_cubemap_textures && !b_generate_irradiance_map) || equirect_map_path.empty())
	{
		std::cout << "Usage: EquirectTool [-c|--generate_cubemap_textures] [-i|--generate_irradiance_map] <equirect_file>" << std::endl;
		return -1;
	}

	{
		Sunset::EquirectToolsApplication app;

		app.init(equirect_map_path, b_generate_cubemap_textures, b_generate_irradiance_map);

		app.run();

		app.cleanup();
	}

	return 0;
}
