#include <graphics/resource/mesh.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>
#include <graphics/asset_pool.h>
#include <graphics/renderer.h>
#include <graphics/command_queue.h>
#include <mesh_serializer.h>

namespace Sunset
{
	void Mesh::upload(GraphicsContext* const gfx_context)
	{
		{
			// Upload vertex buffer
			const size_t vertex_data_size = vertices.size() * sizeof(Vertex);

			std::string buffer_name = name;
			buffer_name += "_vertex_staging";
			Buffer* const vertex_staging_buffer = BufferFactory::create(
				gfx_context,
				{
					.name = buffer_name.c_str(),
					.buffer_size = vertex_data_size,
					.type = BufferType::TransferSource,
					.memory_usage = MemoryUsageType::OnlyCPU
				},
				false
			);

			vertex_staging_buffer->copy_from(gfx_context, vertices.data(), vertex_data_size);

			buffer_name = name;
			buffer_name += "_vertex";
			vertex_buffer = BufferFactory::create(
				gfx_context,
				{
					.name = buffer_name.c_str(),
					.buffer_size = vertex_data_size,
					.type = BufferType::Vertex | BufferType::TransferDestination,
					.memory_usage = MemoryUsageType::OnlyGPU
				}
			);

			gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(gfx_context, [this, gfx_context, vertex_staging_buffer, vertex_data_size](void* command_buffer)
			{
				vertex_buffer->copy_buffer(gfx_context, command_buffer, vertex_staging_buffer, vertex_data_size);
			});

			vertex_staging_buffer->destroy(gfx_context);
			GlobalAssetPools<Buffer>::get()->deallocate(vertex_staging_buffer);
		}

		{
			// Upload index buffer
			const size_t index_data_size = indices.size() * sizeof(uint32_t);

			std::string buffer_name = name;
			buffer_name += "_index_staging";
			Buffer* const index_staging_buffer = BufferFactory::create(
				gfx_context,
				{
					.name = buffer_name.c_str(),
					.buffer_size = index_data_size,
					.type = BufferType::TransferSource,
					.memory_usage = MemoryUsageType::OnlyCPU
				},
				false
			);

			index_staging_buffer->copy_from(gfx_context, indices.data(), index_data_size);

			buffer_name = name;
			buffer_name += "_index";
			index_buffer = BufferFactory::create(
				gfx_context,
				{
					.name = buffer_name.c_str(),
					.buffer_size = index_data_size,
					.type = BufferType::Index | BufferType::TransferDestination,
					.memory_usage = MemoryUsageType::OnlyGPU
				}
			);

			gfx_context->get_command_queue(DeviceQueueType::Graphics)->submit_immediate(gfx_context, [this, gfx_context, index_staging_buffer, index_data_size](void* command_buffer)
			{
				index_buffer->copy_buffer(gfx_context, command_buffer, index_staging_buffer, index_data_size);
			});

			index_staging_buffer->destroy(gfx_context);
			GlobalAssetPools<Buffer>::get()->deallocate(index_staging_buffer);
		}
	}

	void Mesh::destroy(GraphicsContext* const gfx_context)
	{
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
		Identity id = "engine_triangle";
		static MeshID mesh_id = MeshCache::get()->fetch_or_add(id, gfx_context);
		Mesh* const mesh = MeshCache::get()->fetch(mesh_id);

		if (mesh->vertices.size() == 0)
		{
			mesh->vertices.resize(3);

			mesh->vertices[0].position = { 1.0f, 1.0f, 0.0f };
			mesh->vertices[1].position = { -1.0f, 1.0f, 0.0f };
			mesh->vertices[2].position = { 0.0f, -1.0f, 0.0f };

			mesh->vertices[0].color = { 0.0f, 1.0f, 0.0f };
			mesh->vertices[1].color = { 0.0f, 1.0f, 0.0f };
			mesh->vertices[2].color = { 0.0f, 1.0f, 0.0f };

			mesh->name = id;

			mesh->upload(gfx_context);
		}

		return mesh_id;
	}

	Sunset::MeshID MeshFactory::load(class GraphicsContext* const gfx_context, const char* path)
	{
		Identity id = path;
		const MeshID mesh_id = MeshCache::get()->fetch_or_add(id, gfx_context);
		Mesh* const mesh = MeshCache::get()->fetch(mesh_id);

		if (mesh->vertex_buffer == nullptr)
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

			mesh->upload(gfx_context);
		}

		return mesh_id;
	}
}
