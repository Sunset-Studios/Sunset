﻿#include <cooker.h>
#include <image_serializer.h>
#include <mesh_serializer.h>
#include <shader_serializer.h>

#include <iostream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <tiny_obj_loader.h>
#include <shaderc/shaderc.hpp>

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

		std::vector<VertexPNCUTB32> vertices;
		std::vector<uint32_t> indices;

		vertices.reserve(attrib.vertices.size());
		indices.reserve(attrib.vertices.size() * 2);

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

					VertexPNCUTB32 new_vertex;

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

		for (uint32_t i = 0; i < indices.size(); i += 3)
		{
			// Get the vertices of the triangle
			VertexPNCUTB32& v1 = vertices[indices[i]];
			VertexPNCUTB32& v2 = vertices[indices[i + 1]];
			VertexPNCUTB32& v3 = vertices[indices[i + 2]];

			// Calculate edge and delta vectors
			const float e1_x = v2.position[0] - v1.position[0];
			const float e1_y = v2.position[1] - v1.position[1];
			const float e1_z = v2.position[2] - v1.position[2];

			const float e2_x = v3.position[0] - v1.position[0];
			const float e2_y = v3.position[1] - v1.position[1];
			const float e2_z = v3.position[2] - v1.position[2];

			const float delta_uv1_x = v2.uv[0] - v1.uv[0];
			const float delta_uv1_y = v2.uv[1] - v1.uv[1];
			const float delta_uv1_z = v2.uv[2] - v1.uv[2];

			const float delta_uv2_x = v3.uv[0] - v1.uv[0];
			const float delta_uv2_y = v3.uv[1] - v1.uv[1];
			const float delta_uv2_z = v3.uv[2] - v1.uv[2];

			// Compute the tangent and bitangent vectors
			const float f = 1.0f / (delta_uv1_x * delta_uv2_y - delta_uv2_x * delta_uv1_y);

			glm::vec3 tangent;
			tangent.x = f * (delta_uv2_y * e1_x - delta_uv1_y * e2_x);
			tangent.y = f * (delta_uv2_y * e1_y - delta_uv1_y * e2_y);
			tangent.z = f * (delta_uv2_y * e1_z - delta_uv1_y * e2_z);

			glm::vec3 bitangent;
			bitangent.x = f * (-delta_uv2_x * e1_x + delta_uv1_x * e2_x);
			bitangent.y = f * (-delta_uv2_x * e1_y + delta_uv1_x * e2_y);
			bitangent.z = f * (-delta_uv2_x * e1_z + delta_uv1_x * e2_z);

			// Accumulate tangents and bitangents for every vertex of the triangle
			v1.tangent[0] += tangent.x;
			v1.tangent[1] += tangent.y;
			v1.tangent[2] += tangent.z;

			v2.tangent[0] += tangent.x;
			v2.tangent[1] += tangent.y;
			v2.tangent[2] += tangent.z;

			v3.tangent[0] += tangent.x;
			v3.tangent[1] += tangent.y;
			v3.tangent[2] += tangent.z;

			v1.bitangent[0] += bitangent.x;
			v1.bitangent[1] += bitangent.y;
			v1.bitangent[2] += bitangent.z;

			v2.bitangent[0] += bitangent.x;
			v2.bitangent[1] += bitangent.y;
			v2.bitangent[2] += bitangent.z;

			v3.bitangent[0] += bitangent.x;
			v3.bitangent[1] += bitangent.y;
			v3.bitangent[2] += bitangent.z;
		}

		for (uint32_t i = 0; i < vertices.size(); ++i)
		{
			VertexPNCUTB32& vertex = vertices[i];

			glm::vec3 tangent(vertex.tangent[0], vertex.tangent[1], vertex.tangent[2]);
			glm::vec3 bitangent(vertex.bitangent[0], vertex.bitangent[1], vertex.bitangent[2]);
			glm::vec3 normal(vertex.normal[0], vertex.normal[1], vertex.normal[2]);

			tangent = glm::normalize(tangent);
			bitangent = glm::normalize(bitangent);

			// Orthogonalize and normalize the tangent vector using the Gram-Schmidt process
			tangent = glm::normalize(tangent - glm::dot(tangent, normal) * normal);

			vertex.tangent[0] = tangent.x;
			vertex.tangent[1] = tangent.y;
			vertex.tangent[2] = tangent.z;

			vertex.bitangent[0] = bitangent.x;
			vertex.bitangent[1] = bitangent.y;
			vertex.bitangent[2] = bitangent.z;
		}

		SerializedMeshInfo mesh_info;
		mesh_info.format = VertexFormat::PNCUTB32;
		mesh_info.vertex_buffer_size = vertices.size() * sizeof(VertexPNCUTB32);
		mesh_info.index_buffer_size = indices.size() * sizeof(uint32_t);
		mesh_info.index_size = sizeof(uint32_t);
		mesh_info.file_path = input_path.string();
		mesh_info.bounds = calculate_mesh_bounds(vertices.data(), vertices.size());

		SerializedAsset asset = pack_mesh(&mesh_info, (char*)vertices.data(), (char*)indices.data());

		serialize_asset(output_path.string().c_str(), asset);

		return true;
	}

	bool Cooker::cook_shader(const std::filesystem::path& input_path, const std::filesystem::path& output_path)
	{
		std::string shader_string = read_shader_file(input_path);

		parse_shader_includes(shader_string);

		shaderc_shader_kind shader_kind = shaderc_shader_kind::shaderc_glsl_infer_from_source;
		if (input_path.extension() == ".vert")
		{
			shader_kind = shaderc_shader_kind::shaderc_vertex_shader;
		}
		else if (input_path.extension() == ".frag")
		{
			shader_kind = shaderc_shader_kind::shaderc_fragment_shader;
		}
		else if (input_path.extension() == ".comp")
		{
			shader_kind = shaderc_shader_kind::shaderc_compute_shader;
		}

		shaderc::Compiler compiler;
		shaderc::CompileOptions compile_options;

		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(shader_string, shader_kind, (const char*)input_path.c_str(), compile_options);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			// Handle the error
			std::cerr << "Error compiling shader: " << result.GetErrorMessage() << std::endl;
			return false;
		}

		std::vector<uint32_t> compiled_shader_string(result.begin(), result.end());

		SerializedShaderInfo shader_info;
		shader_info.shader_buffer_size = compiled_shader_string.size() * sizeof(uint32_t);
		shader_info.file_path = input_path.string();

		SerializedAsset new_shader_asset = pack_shader(&shader_info, compiled_shader_string.data());

		serialize_asset(output_path.string().c_str(), new_shader_asset);

		return true;
	}
}