#include <cooker.h>

#include <iostream>

int main(int argc, char* argv[])
{
	std::filesystem::path file_path{ argv[1] };
	
	for (auto& p : std::filesystem::directory_iterator(file_path))
	{
		if (p.path().extension() == ".png")
		{
			std::cout << "Cooking texture: " << p << std::endl;
			auto new_path = p.path();
			new_path.replace_extension(".sun");
			Sunset::Cooker::cook_image(p.path(), new_path);
		}
		else if (p.path().extension() == ".obj")
		{
			std::cout << "Cooking mesh: " << p << std::endl;
			auto new_path = p.path();
			new_path.replace_extension(".sun");
			Sunset::Cooker::cook_mesh(p.path(), new_path);
		}
	}
}
