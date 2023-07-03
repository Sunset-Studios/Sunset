#include <graphics/resource/mesh.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>
#include <graphics/asset_pool.h>
#include <graphics/renderer.h>
#include <graphics/command_queue.h>
#include <mesh_serializer.h>

namespace Sunset
{
	void upload_mesh(GraphicsContext* const gfx_context, Mesh* mesh)
	{
		{
			// Upload vertex buffer
			const size_t vertex_data_size = mesh->vertices.size() * sizeof(Vertex);

			std::string buffer_name = std::to_string(mesh->name.computed_hash);
			buffer_name += "_vertex_staging";
			const BufferID vertex_staging_buffer_id = BufferFactory::create(
				gfx_context,
				{
					.name = buffer_name.c_str(),
					.buffer_size = vertex_data_size,
					.type = BufferType::TransferSource,
					.memory_usage = MemoryUsageType::OnlyCPU
				},
				false
			);
			Buffer* const vertex_staging_buffer = CACHE_FETCH(Buffer, vertex_staging_buffer_id);

			vertex_staging_buffer->copy_from(gfx_context, mesh->vertices.data(), vertex_data_size);

			buffer_name = std::to_string(mesh->name.computed_hash);
			buffer_name += "_vertex";
			mesh->vertex_buffer = BufferFactory::create(
				gfx_context,
				{
					.name = buffer_name.c_str(),
					.buffer_size = vertex_data_size,
					.type = BufferType::Vertex | BufferType::TransferDestination,
					.memory_usage = MemoryUsageType::OnlyGPU
				}
			);

			gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(gfx_context, 0, [mesh, gfx_context, vertex_staging_buffer, vertex_data_size](void* command_buffer)
			{
				Buffer* const vertex_buffer_obj = CACHE_FETCH(Buffer, mesh->vertex_buffer);
				vertex_buffer_obj->copy_buffer(gfx_context, command_buffer, vertex_staging_buffer, vertex_data_size);
			});

			CACHE_DELETE(Buffer, vertex_staging_buffer_id, gfx_context);
		}

		for (uint32_t i = 0; i < mesh->sections.size(); ++i)
		{
			MeshSection& section = mesh->sections[i];

			// Upload index buffer
			const size_t index_data_size = section.indices.size() * sizeof(uint32_t);

			std::string buffer_name = std::to_string(mesh->name.computed_hash);
			buffer_name += "_index_staging" + std::to_string(i);
			const BufferID index_staging_buffer_id = BufferFactory::create(
				gfx_context,
				{
					.name = buffer_name.c_str(),
					.buffer_size = index_data_size,
					.type = BufferType::TransferSource,
					.memory_usage = MemoryUsageType::OnlyCPU
				},
				false
			);
			Buffer* const index_staging_buffer = CACHE_FETCH(Buffer, index_staging_buffer_id);

			index_staging_buffer->copy_from(gfx_context, section.indices.data(), index_data_size);

			buffer_name = std::to_string(mesh->name.computed_hash);
			buffer_name += "_index" + std::to_string(i);
			section.index_buffer = BufferFactory::create(
				gfx_context,
				{
					.name = buffer_name.c_str(),
					.buffer_size = index_data_size,
					.type = BufferType::Index | BufferType::TransferDestination,
					.memory_usage = MemoryUsageType::OnlyGPU
				}
			);

			gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(gfx_context, 0, [&section, gfx_context, index_staging_buffer, index_data_size](void* command_buffer)
			{
				Buffer* const index_buffer_obj = CACHE_FETCH(Buffer, section.index_buffer);
				index_buffer_obj->copy_buffer(gfx_context, command_buffer, index_staging_buffer, index_data_size);
			});

			CACHE_DELETE(Buffer, index_staging_buffer_id, gfx_context);
		}
	}

	void destroy_mesh(GraphicsContext* const gfx_context, Mesh* mesh)
	{
	}

	Bounds calculate_mesh_bounds(Mesh* mesh, const glm::mat4& transform)
	{
		Bounds new_bounds;

		const float x_scale = glm::length(transform[0]);
		const float y_scale = glm::length(transform[1]);
		const float z_scale = glm::length(transform[2]);

		float transform_max_scale = glm::max(x_scale, y_scale);
		transform_max_scale = glm::max(transform_max_scale, z_scale);

		new_bounds.extents = mesh->local_bounds.extents * glm::vec3(x_scale, y_scale, z_scale);
		new_bounds.origin = transform * glm::vec4(mesh->local_bounds.origin, 1.0f);
		new_bounds.radius = transform_max_scale * mesh->local_bounds.radius;

		return new_bounds;
	}

	void update_mesh_tangent_bitangents(Mesh* mesh)
	{
		for (MeshSection& section : mesh->sections)
		{
			for (uint32_t i = 0; i < section.indices.size(); i += 3)
			{
				// Get the vertices of the triangle
				Vertex& v1 = mesh->vertices[section.indices[i]];
				Vertex& v2 = mesh->vertices[section.indices[i + 1]];
				Vertex& v3 = mesh->vertices[section.indices[i + 2]];

				// Calculate edge and delta vectors
				const glm::vec3 e1 = v2.position - v1.position;
				const glm::vec3 e2 = v3.position - v1.position;

				const glm::vec2 delta_uv1 = v2.uv - v1.uv;
				const glm::vec2 delta_uv2 = v3.uv - v1.uv;

				// Compute the tangent and bitangent vectors
				const float f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);

				glm::vec3 tangent;
				tangent.x = f * (delta_uv2.y * e1.x - delta_uv1.y * e2.x);
				tangent.y = f * (delta_uv2.y * e1.y - delta_uv1.y * e2.y);
				tangent.z = f * (delta_uv2.y * e1.z - delta_uv1.y * e2.z);

				// Accumulate tangents and bitangents for every vertex of the triangle
				v1.tangent += tangent;
				v2.tangent += tangent;
				v3.tangent += tangent;
			}
		}

		for (uint32_t i = 0; i < mesh->vertices.size(); ++i)
		{
			Vertex& vertex = mesh->vertices[i];

			vertex.tangent = glm::normalize(vertex.tangent);
			// Orthogonalize and normalize the tangent vector using the Gram-Schmidt process
			vertex.tangent = glm::normalize(vertex.tangent - glm::dot(vertex.tangent, vertex.normal) * vertex.normal);
			vertex.bitangent = glm::normalize(glm::cross(vertex.tangent, vertex.normal));
		}
	}

	Sunset::Bounds get_mesh_bounds(MeshID mesh)
	{
		assert(mesh != 0 && "Cannot fetch bounds on null mesh!");
		return CACHE_FETCH(Mesh, mesh)->local_bounds;
	}

	Sunset::PipelineVertexInputDescription Vertex::get_description()
	{
		PipelineVertexInputDescription description;

		description.bindings.emplace_back(0, sizeof(Vertex));

		description.attributes.emplace_back(0, 0, Format::Float3x32, offsetof(Vertex, position));
		description.attributes.emplace_back(0, 1, Format::Float3x32, offsetof(Vertex, normal));
		description.attributes.emplace_back(0, 2, Format::Float3x32, offsetof(Vertex, color));
		description.attributes.emplace_back(0, 3, Format::Float2x32, offsetof(Vertex, uv));
		description.attributes.emplace_back(0, 4, Format::Float3x32, offsetof(Vertex, tangent));
		description.attributes.emplace_back(0, 5, Format::Float3x32, offsetof(Vertex, bitangent));

		return description;
	}

	Sunset::MeshID MeshFactory::create_triangle(class GraphicsContext* const gfx_context)
	{
		Identity id{ "engine_triangle" };
		bool b_added{ false };
		static MeshID mesh_id = MeshCache::get()->fetch_or_add(id, gfx_context, b_added);
		Mesh* const mesh = CACHE_FETCH(Mesh, mesh_id);

		if (b_added)
		{
			mesh->vertices.resize(3);
			MeshSection& section = mesh->sections.emplace_back();

			mesh->vertices[0].position = { 1.0f, 1.0f, 0.0f };
			mesh->vertices[1].position = { -1.0f, 1.0f, 0.0f };
			mesh->vertices[2].position = { 0.0f, -1.0f, 0.0f };

			mesh->vertices[0].normal = { 0.0f, 0.0f, 1.0f };
			mesh->vertices[1].normal = { 0.0f, 0.0f, 1.0f };
			mesh->vertices[2].normal = { 0.0f, 0.0f, 1.0f };

			mesh->vertices[0].color = { 0.0f, 1.0f, 0.0f };
			mesh->vertices[1].color = { 0.0f, 1.0f, 0.0f };
			mesh->vertices[2].color = { 0.0f, 1.0f, 0.0f };

			section.indices.insert(section.indices.end(), { 0, 1, 2 });

			update_mesh_tangent_bitangents(mesh);

			mesh->name = id;

			{
				mesh->local_bounds.extents = glm::vec3(1.0f, 1.0f, 0.0f);
				mesh->local_bounds.origin = glm::vec3(0.0f);
				mesh->local_bounds.radius = 1.0f;
			}

			upload_mesh(gfx_context, mesh);
		}

		return mesh_id;
	}

	Sunset::MeshID MeshFactory::create_quad(class GraphicsContext* const gfx_context)
	{
		Identity id{ "engine_quad" };
		bool b_added{ false };
		static MeshID mesh_id = MeshCache::get()->fetch_or_add(id, gfx_context, b_added);
		Mesh* const mesh = CACHE_FETCH(Mesh, mesh_id);

		if (b_added)
		{
			mesh->vertices.resize(4);
			MeshSection& section = mesh->sections.emplace_back();

			mesh->vertices[0].position = { -1.0f, 1.0f, 0.0f };
			mesh->vertices[1].position = { -1.0f, -1.0f, 0.0f };
			mesh->vertices[2].position = { 1.0f, -1.0f, 0.0f };
			mesh->vertices[3].position = { 1.0f, 1.0f, 0.0f };

			mesh->vertices[0].normal = { 0.0f, 0.0f, -1.0f };
			mesh->vertices[1].normal = { 0.0f, 0.0f, -1.0f };
			mesh->vertices[2].normal = { 0.0f, 0.0f, -1.0f };
			mesh->vertices[3].normal = { 0.0f, 0.0f, -1.0f };

			mesh->vertices[0].uv = { 0.0f, 1.0f };
			mesh->vertices[1].uv = { 0.0f, 0.0f };
			mesh->vertices[2].uv = { 1.0f, 0.0f };
			mesh->vertices[3].uv = { 1.0f, 1.0f };

			section.indices.insert(section.indices.end(), { 0, 1, 2, 0, 2, 3 });

			update_mesh_tangent_bitangents(mesh);

			mesh->name = id;

			{
				mesh->local_bounds.extents = glm::vec3(1.0f, 1.0f, 0.0f);
				mesh->local_bounds.origin = glm::vec3(0.0f);
				mesh->local_bounds.radius = 1.0f;
			}

			upload_mesh(gfx_context, mesh);
		}

		return mesh_id;
	}

	Sunset::MeshID MeshFactory::create_sphere(class GraphicsContext* const gfx_context, const glm::ivec2& segments, float radius)
	{
		Identity id{ "engine_sphere" };
		bool b_added{ false };
		static MeshID mesh_id = MeshCache::get()->fetch_or_add(id, gfx_context, b_added);
		Mesh* const mesh = CACHE_FETCH(Mesh, mesh_id);

		if (b_added)
		{
			float x, y, z;
			float xy;
			float nx, ny, nz;
			float lengthInv = 1.0f / radius;
			float s, t;

			const uint32_t sectorCount = segments.x;
			const uint32_t stackCount = segments.y;

			const float sectorStep = TWOPI / sectorCount;
			const float stackStep = PI / stackCount;

			float sectorAngle, stackAngle;

			MeshSection& section = mesh->sections.emplace_back();

			for (uint32_t i = 0; i <= stackCount; ++i)
			{
				stackAngle = PI * 0.5f - i * stackStep;
				xy = radius * cosf(stackAngle);
				z = radius * sinf(stackAngle);

				for (uint32_t j = 0; j <= sectorCount; ++j)
				{
					sectorAngle = j * sectorStep;

					x = xy * cosf(sectorAngle);
					y = xy * sinf(sectorAngle);

					mesh->vertices.emplace_back(
						glm::vec3(x, y, z),										 // position
						glm::normalize(glm::vec3(x, y, z)),					     // normal
						glm::vec3(1.0f, 1.0f, 1.0f),							 // color
						glm::vec2(static_cast<float>(j) / static_cast<float>(sectorCount), static_cast<float>(i) / static_cast<float>(stackCount)) // uv
					);
				}
			}

			uint32_t k1, k2;
			for (uint32_t i = 0; i < stackCount; ++i)
			{
				k1 = i * (sectorCount + 1);
				k2 = k1 + sectorCount + 1;

				for (uint32_t j = 0; j < sectorCount; ++j, ++k1, ++k2)
				{
					if (i != 0)
					{
						section.indices.insert(section.indices.end(),
							{ k1, k2, k1 + 1 }
						);
					}

					if (i != (stackCount - 1))
					{
						section.indices.insert(section.indices.end(),
							{ k1 + 1, k2, k2 + 1 }
						);
					}
				}
			}

			update_mesh_tangent_bitangents(mesh);

			mesh->name = id;

			{
				mesh->local_bounds.extents = glm::vec3(1.0f, 1.0f, 1.0f);
				mesh->local_bounds.origin = glm::vec3(0.0f);
				mesh->local_bounds.radius = 1.0f;
			}

			upload_mesh(gfx_context, mesh);
		}

		return mesh_id;
	}

	MeshID MeshFactory::create_cube(GraphicsContext* const gfx_context)
	{
		Identity id{ "engine_cube" };
		bool b_added{ false };
		static MeshID mesh_id = MeshCache::get()->fetch_or_add(id, gfx_context, b_added);
		Mesh* const mesh = CACHE_FETCH(Mesh, mesh_id);

		if (b_added)
		{
			mesh->vertices.resize(24);
			MeshSection& section = mesh->sections.emplace_back();

			// Front face
			mesh->vertices[0].position = { -0.5f, -0.5f, 0.5f }; mesh->vertices[0].normal = { 0.0f, 0.0f, 1.0f }; mesh->vertices[0].uv = { 0.0f, 0.0f };

			mesh->vertices[1].position = { 0.5f, -0.5f, 0.5f }; mesh->vertices[1].normal = { 0.0f, 0.0f, 1.0f }; mesh->vertices[1].uv = { 1.0f, 0.0f };

			mesh->vertices[2].position = { 0.5f, 0.5f, 0.5f }; mesh->vertices[2].normal = { 0.0f, 0.0f, 1.0f }; mesh->vertices[2].uv = { 1.0f, 1.0f };

			mesh->vertices[3].position = { -0.5f, 0.5f, 0.5f }; mesh->vertices[3].normal = { 0.0f, 0.0f, 1.0f }; mesh->vertices[3].uv = { 0.0f, 1.0f };

			// Back face
			mesh->vertices[4].position = { -0.5f, -0.5f, -0.5f }; mesh->vertices[4].normal = { 0.0f, 0.0f, -1.0f }; mesh->vertices[4].uv = { 1.0f, 0.0f };

			mesh->vertices[5].position = { 0.5f, -0.5f, -0.5f }; mesh->vertices[5].normal = { 0.0f, 0.0f, -1.0f }; mesh->vertices[5].uv = { 0.0f, 0.0f };

			mesh->vertices[6].position = { 0.5f, 0.5f, -0.5f }; mesh->vertices[6].normal = { 0.0f, 0.0f, -1.0f }; mesh->vertices[6].uv = { 0.0f, 1.0f };

			mesh->vertices[7].position = { -0.5f, 0.5f, -0.5f }; mesh->vertices[7].normal = { 0.0f, 0.0f, -1.0f }; mesh->vertices[7].uv = { 1.0f, 1.0f };

			// Left face
			mesh->vertices[8].position = { -0.5f, -0.5f, -0.5f }; mesh->vertices[8].normal = { -1.0f, 0.0f, 0.0f }; mesh->vertices[8].uv = { 1.0f, 0.0f };

			mesh->vertices[9].position = { -0.5f, -0.5f, 0.5f }; mesh->vertices[9].normal = { -1.0f, 0.0f, 0.0f }; mesh->vertices[9].uv = { 0.0f, 0.0f };

			mesh->vertices[10].position = { -0.5f, 0.5f, 0.5f }; mesh->vertices[10].normal = { -1.0f, 0.0f, 0.0f }; mesh->vertices[10].uv = { 0.0f, 1.0f };

			mesh->vertices[11].position = { -0.5f, 0.5f, -0.5f }; mesh->vertices[11].normal = { -1.0f, 0.0f, 0.0f }; mesh->vertices[11].uv = { 1.0f, 1.0f };

			// Right face
			mesh->vertices[12].position = { 0.5f, -0.5f, -0.5f }; mesh->vertices[12].normal = { 1.0f, 0.0f, 0.0f }; mesh->vertices[12].uv = { 0.0f, 0.0f };

			mesh->vertices[13].position = { 0.5f, -0.5f, 0.5f }; mesh->vertices[13].normal = { 1.0f, 0.0f, 0.0f }; mesh->vertices[13].uv = { 1.0f, 0.0f };

			mesh->vertices[14].position = { 0.5f, 0.5f, 0.5f }; mesh->vertices[14].normal = { 1.0f, 0.0f, 0.0f }; mesh->vertices[14].uv = { 1.0f, 1.0f };

			mesh->vertices[15].position = { 0.5f, 0.5f, -0.5f }; mesh->vertices[15].normal = { 1.0f, 0.0f, 0.0f }; mesh->vertices[15].uv = { 0.0f, 1.0f };

			// Bottom face
			mesh->vertices[16].position = { -0.5f, -0.5f, -0.5f }; mesh->vertices[16].normal = { 0.0f, -1.0f, 0.0f }; mesh->vertices[16].uv = { 1.0f, 0.0f };

			mesh->vertices[17].position = { 0.5f, -0.5f, -0.5f }; mesh->vertices[17].normal = { 0.0f, -1.0f, 0.0f }; mesh->vertices[17].uv = { 0.0f, 0.0f };

			mesh->vertices[18].position = { 0.5f, -0.5f, 0.5f }; mesh->vertices[18].normal = { 0.0f, -1.0f, 0.0f }; mesh->vertices[18].uv = { 0.0f, 1.0f };

			mesh->vertices[19].position = { -0.5f, -0.5f, 0.5f }; mesh->vertices[19].normal = { 0.0f, -1.0f, 0.0f }; mesh->vertices[19].uv = { 1.0f, 1.0f };

			// Top face
			mesh->vertices[20].position = { -0.5f, 0.5f, -0.5f }; mesh->vertices[20].normal = { 0.0f, 1.0f, 0.0f }; mesh->vertices[20].uv = { 1.0f, 0.0f };

			mesh->vertices[21].position = { 0.5f, 0.5f, -0.5f }; mesh->vertices[21].normal = { 0.0f, 1.0f, 0.0f }; mesh->vertices[21].uv = { 0.0f, 0.0f };

			mesh->vertices[22].position = { 0.5f, 0.5f, 0.5f }; mesh->vertices[22].normal = { 0.0f, 1.0f, 0.0f }; mesh->vertices[22].uv = { 0.0f, 1.0f };

			mesh->vertices[23].position = { -0.5f, 0.5f, 0.5f }; mesh->vertices[23].normal = { 0.0f, 1.0f, 0.0f }; mesh->vertices[23].uv = { 1.0f, 1.0f };

			section.indices.insert(section.indices.end(),
				{   
					// Front face
					0, 1, 2, 0, 2, 3,
					// Back face
					4, 5, 6, 4, 6, 7,
					// Left face
					8, 9, 10, 8, 10, 11,
					// Right face
					12, 13, 14, 12, 14, 15,
					// Bottom face
					16, 17, 18, 16, 18, 19,
					// Top face
					20, 21, 22, 20, 22, 23
				}
			);

			update_mesh_tangent_bitangents(mesh);

			mesh->name = id;

			{
				mesh->local_bounds.extents = glm::vec3(0.5f, 0.5f, 0.5f);
				mesh->local_bounds.origin = glm::vec3(0.0f);
				mesh->local_bounds.radius = 1.0f;
			}

			upload_mesh(gfx_context, mesh);
		}

		return mesh_id;
	}

	Sunset::MeshID MeshFactory::load(class GraphicsContext* const gfx_context, const char* path)
	{
		Identity id{ path };
		bool b_added{ false };
		const MeshID mesh_id = MeshCache::get()->fetch_or_add(id, gfx_context, b_added);
		Mesh* const mesh = CACHE_FETCH(Mesh, mesh_id);

		if (b_added)
		{
			SerializedAsset asset;
			if (!deserialize_asset(path, asset))
			{
				// TODO: Need some custom logging
				return 0;
			}

			SerializedMeshInfo serialized_mesh_info = get_serialized_mesh_info(&asset);

			std::vector<char> vertex_buffer;
			std::vector<char> index_buffer;

			vertex_buffer.resize(serialized_mesh_info.vertex_buffer_size);
			index_buffer.resize(serialized_mesh_info.index_buffer_size);

			unpack_mesh(&serialized_mesh_info, asset.binary.data(), asset.binary.size(), vertex_buffer.data(), index_buffer.data());

			mesh->vertices.clear();

			if (serialized_mesh_info.format == VertexFormat::PNCU32)
			{
				VertexPNCU32* unpacked_vertices = (VertexPNCU32*)vertex_buffer.data();
				mesh->vertices.resize(vertex_buffer.size() / sizeof(VertexPNCU32));

				for (int i = 0; i < mesh->vertices.size(); ++i)
				{
					mesh->vertices[i].position.x = unpacked_vertices[i].position[0];
					mesh->vertices[i].position.y = unpacked_vertices[i].position[1];
					mesh->vertices[i].position.z = unpacked_vertices[i].position[2];

					mesh->vertices[i].normal.x = unpacked_vertices[i].normal[0];
					mesh->vertices[i].normal.y = unpacked_vertices[i].normal[1];
					mesh->vertices[i].normal.z = unpacked_vertices[i].normal[2];

					mesh->vertices[i].color.r = mesh->vertices[i].color[0];
					mesh->vertices[i].color.g = mesh->vertices[i].color[1];
					mesh->vertices[i].color.b = mesh->vertices[i].color[2];

					mesh->vertices[i].uv.x = unpacked_vertices[i].uv[0];
					mesh->vertices[i].uv.y = unpacked_vertices[i].uv[1];
				}
			}
			else if (serialized_mesh_info.format == VertexFormat::PNCUTB32)
			{
				VertexPNCUTB32* unpacked_vertices = (VertexPNCUTB32*)vertex_buffer.data();
				mesh->vertices.resize(vertex_buffer.size() / sizeof(VertexPNCUTB32));

				for (int i = 0; i < mesh->vertices.size(); ++i)
				{
					mesh->vertices[i].position.x = unpacked_vertices[i].position[0];
					mesh->vertices[i].position.y = unpacked_vertices[i].position[1];
					mesh->vertices[i].position.z = unpacked_vertices[i].position[2];

					mesh->vertices[i].normal.x = unpacked_vertices[i].normal[0];
					mesh->vertices[i].normal.y = unpacked_vertices[i].normal[1];
					mesh->vertices[i].normal.z = unpacked_vertices[i].normal[2];

					mesh->vertices[i].color.r = mesh->vertices[i].color[0];
					mesh->vertices[i].color.g = mesh->vertices[i].color[1];
					mesh->vertices[i].color.b = mesh->vertices[i].color[2];

					mesh->vertices[i].uv.x = unpacked_vertices[i].uv[0];
					mesh->vertices[i].uv.y = unpacked_vertices[i].uv[1];

					mesh->vertices[i].tangent.x = unpacked_vertices[i].tangent[0];
					mesh->vertices[i].tangent.y = unpacked_vertices[i].tangent[1];
					mesh->vertices[i].tangent.z = unpacked_vertices[i].tangent[2];

					mesh->vertices[i].bitangent.x = unpacked_vertices[i].bitangent[0];
					mesh->vertices[i].bitangent.y = unpacked_vertices[i].bitangent[1];
					mesh->vertices[i].bitangent.z = unpacked_vertices[i].bitangent[2];
				}
			}
			else if (serialized_mesh_info.format == VertexFormat::P32N8C8U16)
			{
				VertexP32N8C8U16* unpacked_vertices = (VertexP32N8C8U16*)vertex_buffer.data();
				mesh->vertices.resize(vertex_buffer.size() / sizeof(VertexP32N8C8U16));

				for (int i = 0; i < mesh->vertices.size(); ++i)
				{
					mesh->vertices[i].position.x = unpacked_vertices[i].position[0];
					mesh->vertices[i].position.y = unpacked_vertices[i].position[1];
					mesh->vertices[i].position.z = unpacked_vertices[i].position[2];

					mesh->vertices[i].normal.x = unpacked_vertices[i].normal_color[0] & 0xFFFFFF00;
					mesh->vertices[i].normal.y = unpacked_vertices[i].normal_color[1] & 0xFFFFFF00;
					mesh->vertices[i].normal.z = unpacked_vertices[i].normal_color[2] & 0xFFFFFF00;

					mesh->vertices[i].color.r = unpacked_vertices[i].normal_color[0] & 0xFF;
					mesh->vertices[i].color.g = unpacked_vertices[i].normal_color[1] & 0xFF;
					mesh->vertices[i].color.b = unpacked_vertices[i].normal_color[2] & 0xFF;

					mesh->vertices[i].uv.x = unpacked_vertices[i].uv[0];
					mesh->vertices[i].uv.y = unpacked_vertices[i].uv[1];
				}
			}

			uint32_t* unpacked_indices = (uint32_t*)index_buffer.data();

			mesh->sections.reserve(serialized_mesh_info.mesh_section_infos.size());
			for (MeshSectionInfo& section_info : serialized_mesh_info.mesh_section_infos)
			{
				MeshSection& section = mesh->sections.emplace_back();

				section.indices.clear();
				section.indices.resize(section_info.index_buffer_count);
				for (int i = 0; i < section.indices.size(); ++i)
				{
					section.indices[i] = unpacked_indices[section_info.index_buffer_start + i];
				}
			}

			mesh->name = id;

			{
				mesh->local_bounds.extents.x = serialized_mesh_info.bounds.extents[0];
				mesh->local_bounds.extents.y = serialized_mesh_info.bounds.extents[1];
				mesh->local_bounds.extents.z = serialized_mesh_info.bounds.extents[2];
				mesh->local_bounds.origin.x = serialized_mesh_info.bounds.origin[0];
				mesh->local_bounds.origin.y = serialized_mesh_info.bounds.origin[1];
				mesh->local_bounds.origin.z = serialized_mesh_info.bounds.origin[2];
				mesh->local_bounds.radius = serialized_mesh_info.bounds.radius;
			}

			upload_mesh(gfx_context, mesh);
		}

		return mesh_id;
	}
}
