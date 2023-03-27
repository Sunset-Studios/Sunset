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

			gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(gfx_context, [mesh, gfx_context, vertex_staging_buffer, vertex_data_size](void* command_buffer)
			{
				Buffer* const vertex_buffer_obj = CACHE_FETCH(Buffer, mesh->vertex_buffer);
				vertex_buffer_obj->copy_buffer(gfx_context, command_buffer, vertex_staging_buffer, vertex_data_size);
			});

			CACHE_DELETE(Buffer, vertex_staging_buffer_id, gfx_context);
		}

		{
			// Upload index buffer
			const size_t index_data_size = mesh->indices.size() * sizeof(uint32_t);

			std::string buffer_name = std::to_string(mesh->name.computed_hash);
			buffer_name += "_index_staging";
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

			index_staging_buffer->copy_from(gfx_context, mesh->indices.data(), index_data_size);

			buffer_name = std::to_string(mesh->name.computed_hash);
			buffer_name += "_index";
			mesh->index_buffer = BufferFactory::create(
				gfx_context,
				{
					.name = buffer_name.c_str(),
					.buffer_size = index_data_size,
					.type = BufferType::Index | BufferType::TransferDestination,
					.memory_usage = MemoryUsageType::OnlyGPU
				}
			);

			gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(gfx_context, [mesh, gfx_context, index_staging_buffer, index_data_size](void* command_buffer)
			{
				Buffer* const index_buffer_obj = CACHE_FETCH(Buffer, mesh->index_buffer);
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

			mesh->vertices[0].position = { 1.0f, 1.0f, 0.0f };
			mesh->vertices[1].position = { -1.0f, 1.0f, 0.0f };
			mesh->vertices[2].position = { 0.0f, -1.0f, 0.0f };

			mesh->vertices[0].color = { 0.0f, 1.0f, 0.0f };
			mesh->vertices[1].color = { 0.0f, 1.0f, 0.0f };
			mesh->vertices[2].color = { 0.0f, 1.0f, 0.0f };

			mesh->indices.insert(mesh->indices.end(), { 0, 1, 2 });

			mesh->name = id;

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

			mesh->vertices[0].position = { -1.0f, 1.0f, 0.0f };
			mesh->vertices[1].position = { -1.0f, -1.0f, 0.0f };
			mesh->vertices[2].position = { 1.0f, -1.0f, 0.0f };
			mesh->vertices[3].position = { 1.0f, 1.0f, 0.0f };

			mesh->vertices[0].uv = { 0.0f, 1.0f };
			mesh->vertices[1].uv = { 0.0f, 0.0f };
			mesh->vertices[2].uv = { 1.0f, 0.0f };
			mesh->vertices[3].uv = { 1.0f, 1.0f };

			mesh->indices.insert(mesh->indices.end(), { 0, 1, 2, 0, 2, 3 });

			mesh->name = id;

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
						glm::vec3(x, y, z),
						glm::vec3(x * lengthInv, y * lengthInv, z * lengthInv),
						glm::vec3(1.0f, 1.0f, 1.0f),
						glm::vec2((float)j / sectorCount, (float)i / stackCount)
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
						mesh->indices.insert(mesh->indices.end(),
							{ k1, k2, k1 + 1 }
						);
					}

					if (i != (stackCount - 1))
					{
						mesh->indices.insert(mesh->indices.end(),
							{ k1 + 1, k2, k2 + 1 }
						);
					}
				}
			}

			mesh->name = id;

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

			mesh->indices.clear();

			uint32_t* unpacked_indices = (uint32_t*)index_buffer.data();
			mesh->indices.resize(index_buffer.size() / sizeof(uint32_t));

			for (int i = 0; i < mesh->indices.size(); ++i)
			{
				mesh->indices[i] = unpacked_indices[i];
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
