#include <cooker.h>
#include <image_serializer.h>
#include <mesh_serializer.h>

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <tiny_obj_loader.h>

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
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string warn;
		std::string err;
		tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, input_path.string().c_str(), nullptr);

		if (!err.empty())
		{
			return false;
		}

		std::vector<VertexPNCU32> vertices;
		std::vector<uint32_t> indices;

		const int32_t face_vertices = 3;
		for (size_t s = 0; s < shapes.size(); ++s)
		{
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f)
			{
				for (size_t v = 0; v < face_vertices; ++v)
				{
					tinyobj::index_t index = shapes[s].mesh.indices[index_offset + v];

					// Position
					tinyobj::real_t vx = attrib.vertices[3 * index.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * index.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * index.vertex_index + 2];

					// Normal
					tinyobj::real_t nx = attrib.normals[3 * index.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * index.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * index.normal_index + 2];

					// UVs
					tinyobj::real_t ux = index.texcoord_index >= 0 ? attrib.texcoords[2 * index.texcoord_index + 0] : 0;
					tinyobj::real_t uy = index.texcoord_index >= 0 ? attrib.texcoords[2 * index.texcoord_index + 1] : 0;

					VertexPNCU32 new_vertex;

					new_vertex.position[0] = vx;
					new_vertex.position[1] = vy;
					new_vertex.position[2] = vz;

					new_vertex.normal[0] = nx;
					new_vertex.normal[1] = ny;
					new_vertex.normal[2] = nz;

					new_vertex.color[0] = 0.0f;
					new_vertex.color[1] = 0.0f;
					new_vertex.color[2] = 0.0f;

					new_vertex.uv[0] = ux;
					new_vertex.uv[1] = 1 - uy;

					indices.push_back(vertices.size());
					vertices.push_back(new_vertex);
				}

				index_offset += face_vertices;
			}
		}

		SerializedMeshInfo mesh_info;
		mesh_info.format = VertexFormat::PNCU32;
		mesh_info.vertex_buffer_size = vertices.size() * sizeof(VertexPNCU32);
		mesh_info.index_buffer_size = indices.size() * sizeof(uint32_t);
		mesh_info.index_size = sizeof(uint32_t);
		mesh_info.file_path = input_path.string();
		mesh_info.bounds = calculate_mesh_bounds(vertices.data(), vertices.size());

		SerializedAsset asset = pack_mesh(&mesh_info, (char*)vertices.data(), (char*)indices.data());

		serialize_asset(output_path.string().c_str(), asset);

		return true;
	}
}
