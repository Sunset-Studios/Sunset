#include <equirect_tools.h>

#include <iostream>
#include <filesystem>

int main(int argc, char* argv[])
{
	bool b_generate_cubemap_textures = false;
	bool b_generate_irradiance_map = false;
	bool b_generate_prefilter_map = false;
	bool b_generate_brdf_lut = false;
	bool b_flip_writes = true;
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
		else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--generate_prefilter_map") == 0)
		{
			b_generate_prefilter_map = true;
		}
		else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--generate_brdf_lut") == 0)
		{
			b_generate_brdf_lut = true;
		}
		else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--dont_flip_writes") == 0)
		{
			b_flip_writes = false;
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

	if ((!b_generate_cubemap_textures && !b_generate_irradiance_map && !b_generate_prefilter_map && !b_generate_brdf_lut) || equirect_map_path.empty())
	{
		std::cout << "Usage: EquirectTool [-c|--generate_cubemap_textures] [-i|--generate_irradiance_map] "
				  << "[-p | --generate_prefilter_map][-b | --generate_brdf_lut][-f | --dont_flip_writes] <equirect_file>"
				  << std::endl;
		return -1;
	}

	{
		Sunset::EquirectToolsApplication app;

		app.init(equirect_map_path, b_generate_cubemap_textures, b_generate_irradiance_map, b_generate_prefilter_map, b_generate_brdf_lut, b_flip_writes);

		app.run();

		app.cleanup();
	}

	return 0;
}
