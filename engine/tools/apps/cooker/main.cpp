#include <cooker.h>

#include <iostream>

int main(int argc, char* argv[])
{
	for (uint32_t i = 1; i < argc; ++i)
	{
		std::filesystem::path file_path{ argv[i] };

		for (auto& p : std::filesystem::recursive_directory_iterator(file_path))
		{
			const std::filesystem::path extension = p.path().extension();

			if (extension == ".png")
			{
				std::cout << "Cooking texture: " << p << std::endl;
				auto new_path = p.path();
				new_path.replace_extension(".sun");
				Sunset::Cooker::cook_image(p.path(), new_path);
			}
			else if (extension == ".obj")
			{
				std::cout << "Cooking mesh: " << p << std::endl;
				auto new_path = p.path();
				new_path.replace_extension(".sun");
				Sunset::Cooker::cook_mesh_obj(p.path(), new_path);
			}
			else if (extension == ".frag" || extension == ".vert" || extension == ".comp")
			{
				std::cout << "Cooking shader: " << p << std::endl;
				auto new_path = p.path();
				new_path += ".sun";
				Sunset::Cooker::cook_shader(p.path(), new_path);
			}
		}
	}
}
