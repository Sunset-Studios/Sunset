#include <cooker.h>
#include <image_serializer.h>
#include <mesh_serializer.h>
#include <shader_serializer.h>
#include <maths.h>

#include <iostream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include <stb_image.h>
#include <stb_image_resize.h>
#include <tiny_obj_loader.h>
#include <shaderc/shaderc.hpp>
#include <ofbx.h>

namespace Sunset
{
	bool Cooker::cook_image(const std::filesystem::path& input_path, const std::filesystem::path& output_path)
	{
		int texture_width, texture_height, texture_channels;

		int file_size = stbi_info((const char*)input_path.u8string().c_str(), &texture_width, &texture_height, &texture_channels);

		stbi_uc* pixel_buffer = stbi_load((const char*)input_path.u8string().c_str(), &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);

		if (pixel_buffer == nullptr)
		{
			std::cout << "Failed to load image file " << input_path << std::endl;
		}
		
		const bool b_is_pre_mipped = input_path.string().find("mip_") != std::string::npos;

		SerializedImageInfo image_info;
		image_info.size = texture_width * texture_height * 4;
		image_info.extent[0] = texture_width;
		image_info.extent[1] = texture_height;
		image_info.extent[2] = 1;
		image_info.format = Format::UNorm4x8;
		image_info.file_path = input_path.string();
		image_info.channels = 4;
		image_info.mips = b_is_pre_mipped ? 1 : glm::floor(glm::log2(glm::max(static_cast<float>(image_info.extent[0]), static_cast<float>(image_info.extent[1])))) + 1;

		nlohmann::json image_metadata;
		SerializedAsset new_image_asset = pack_image_begin(&image_info, image_metadata);

		size_t total_compressed_buffer_size = pack_image_mip(&image_info, new_image_asset, pixel_buffer, image_metadata, 0, image_info.extent[0], image_info.extent[1]);

		{
			const size_t pot_width = Maths::ppot(texture_width);
			const size_t pot_height = Maths::ppot(texture_height);

			std::vector<unsigned char> mip_pixel_buffer;
			mip_pixel_buffer.resize(image_info.size, 0);

			for (uint32_t m = 1; m < image_info.mips; ++m)
			{
				const size_t mip_width = glm::clamp(pot_width >> m, size_t(1), pot_width);
				const size_t mip_height = glm::clamp(pot_height >> m, size_t(1), pot_height);
				stbir_resize_uint8(pixel_buffer, texture_width, texture_height, 0, mip_pixel_buffer.data(), mip_width, mip_height, 0, image_info.channels);
				total_compressed_buffer_size += pack_image_mip(&image_info, new_image_asset, mip_pixel_buffer.data(), image_metadata, m, mip_width, mip_height, total_compressed_buffer_size);
				std::fill(mip_pixel_buffer.begin(), mip_pixel_buffer.end(), 0);
			}
		}

		pack_image_end(new_image_asset, image_metadata, total_compressed_buffer_size);

		stbi_image_free(pixel_buffer);

		serialize_asset(output_path.string().c_str(), new_image_asset);

		return true;
	}

	bool Cooker::cook_mesh_obj(const std::filesystem::path& input_path, const std::filesystem::path& output_path)
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
		mesh_info.mesh_section_infos.emplace_back(indices.size(), 0);

		SerializedAsset asset = pack_mesh(&mesh_info, (char*)vertices.data(), (char*)indices.data());

		serialize_asset(output_path.string().c_str(), asset);

		return true;
	}

	bool Cooker::cook_mesh_fbx(const std::filesystem::path& input_path, const std::filesystem::path& output_path)
	{
		std::vector<char> fbx_buffer{ read_mesh_file(input_path) };

		ofbx::LoadFlags flags =
			ofbx::LoadFlags::TRIANGULATE |
			ofbx::LoadFlags::IGNORE_BLEND_SHAPES |
			ofbx::LoadFlags::IGNORE_CAMERAS |
			ofbx::LoadFlags::IGNORE_LIGHTS |
			ofbx::LoadFlags::IGNORE_SKIN |
			ofbx::LoadFlags::IGNORE_BONES |
			ofbx::LoadFlags::IGNORE_PIVOTS |
			ofbx::LoadFlags::IGNORE_POSES |
			ofbx::LoadFlags::IGNORE_VIDEOS |
			ofbx::LoadFlags::IGNORE_LIMBS |
			ofbx::LoadFlags::IGNORE_ANIMATIONS;


		if (ofbx::IScene* scene = ofbx::load((ofbx::u8*)fbx_buffer.data(), fbx_buffer.size(), (ofbx::u16)flags))
		{
			SerializedMeshInfo mesh_info;
			mesh_info.format = VertexFormat::PNCUTB32;
			mesh_info.index_size = sizeof(uint32_t);
			mesh_info.file_path = input_path.string();

			uint32_t num_vertices{ 0 };
			uint32_t num_indices{ 0 };
			for (uint32_t g = 0; g < scene->getGeometryCount(); ++g)
			{
				num_vertices += scene->getGeometry(g)->getVertexCount();
				num_indices += scene->getGeometry(g)->getIndexCount();
			}

			std::vector<VertexPNCUTB32> vertices;
			std::vector<uint32_t> indices;

			vertices.reserve(num_vertices);
			indices.reserve(num_indices);

			uint32_t mesh_section_index_start{ 0 };
			uint32_t mesh_section_index_count{ 0 };

			for (uint32_t i = 0; i < scene->getMeshCount(); ++i)
			{
				const ofbx::Mesh* mesh = scene->getMesh(i);

				const ofbx::Matrix fbx_gm = mesh->getGlobalTransform();
				const glm::mat4 world_matrix(
					fbx_gm.m[0], fbx_gm.m[1], fbx_gm.m[2], fbx_gm.m[3],
					fbx_gm.m[4], fbx_gm.m[5], fbx_gm.m[6], fbx_gm.m[7],
					fbx_gm.m[8], fbx_gm.m[9], fbx_gm.m[10], fbx_gm.m[11],
					fbx_gm.m[12], fbx_gm.m[13], fbx_gm.m[14], fbx_gm.m[15]
				);
				const ofbx::Matrix fbx_gmm = mesh->getGeometricMatrix();
				const glm::mat4 geometric_matrix(
					fbx_gmm.m[0], fbx_gmm.m[1], fbx_gmm.m[2], fbx_gmm.m[3],
					fbx_gmm.m[4], fbx_gmm.m[5], fbx_gmm.m[6], fbx_gmm.m[7],
					fbx_gmm.m[8], fbx_gmm.m[9], fbx_gmm.m[10], fbx_gmm.m[11],
					fbx_gmm.m[12], fbx_gmm.m[13], fbx_gmm.m[14], fbx_gmm.m[15]
				);

				const ofbx::Geometry* mesh_geo = mesh->getGeometry();

				const uint32_t vertex_count = mesh_geo->getVertexCount();
				const uint32_t index_count = mesh_geo->getIndexCount();

				const ofbx::Vec3* fbx_vertices = mesh_geo->getVertices();
				const ofbx::Vec3* fbx_normals = mesh_geo->getNormals();
				const ofbx::Vec4* fbx_colors = mesh_geo->getColors();
				const ofbx::Vec2* fbx_uvs = mesh_geo->getUVs();

				mesh_section_index_count = vertex_count;
				mesh_section_index_start = indices.size();

				for (uint32_t v = 0; v < vertex_count; ++v)
				{
					VertexPNCUTB32 new_vertex;

					glm::vec4 vertex_pos(fbx_vertices[v].x, fbx_vertices[v].y, fbx_vertices[v].z, 1.0f);
					vertex_pos = world_matrix * geometric_matrix * vertex_pos;
					
					new_vertex.position[0] = vertex_pos.x;
					new_vertex.position[1] = vertex_pos.y;
					new_vertex.position[2] = vertex_pos.z;

					if (fbx_normals != nullptr)
					{
						glm::vec4 vertex_normal(fbx_normals[v].x, fbx_normals[v].y, fbx_normals[v].z, 1.0f);
						vertex_normal = glm::normalize(glm::inverse(glm::transpose(world_matrix)) * vertex_normal);

						new_vertex.normal[0] = vertex_normal.x;
						new_vertex.normal[1] = vertex_normal.y;
						new_vertex.normal[2] = vertex_normal.z;
					}

					if (fbx_colors != nullptr)
					{
						new_vertex.color[0] = fbx_colors[v].x;
						new_vertex.color[1] = fbx_colors[v].y;
						new_vertex.color[2] = fbx_colors[v].z;
					}

					if (fbx_uvs != nullptr)
					{
						new_vertex.uv[0] = fbx_uvs[v].x;
						new_vertex.uv[1] = 1.0f - fbx_uvs[v].y;
					}

					indices.push_back(vertices.size());
					vertices.push_back(new_vertex);
				}

				mesh_info.mesh_section_infos.emplace_back(mesh_section_index_count, mesh_section_index_start);
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

			mesh_info.vertex_buffer_size = vertices.size() * sizeof(VertexPNCUTB32);
			mesh_info.index_buffer_size = indices.size() * sizeof(uint32_t);
			mesh_info.bounds = calculate_mesh_bounds(vertices.data(), vertices.size());

			SerializedAsset asset = pack_mesh(&mesh_info, (char*)vertices.data(), (char*)indices.data());

			serialize_asset(output_path.string().c_str(), asset);

			return true;
		}
		return false;
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
