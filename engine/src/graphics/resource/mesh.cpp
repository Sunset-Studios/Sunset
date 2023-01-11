#include <graphics/resource/mesh.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>
#include <graphics/asset_pool.h>
#include <graphics/renderer.h>
#include <mesh_serializer.h>

namespace Sunset
{
	void Mesh::upload(GraphicsContext* const gfx_context)
	{
		const size_t vertex_data_size = vertices.size() * sizeof(Vertex);

		Buffer* const staging_buffer = BufferFactory::create(gfx_context, vertex_data_size, BufferType::TransferSource, MemoryUsageType::OnlyCPU, false);

		staging_buffer->copy_from(gfx_context, vertices.data(), vertex_data_size);

		vertex_buffer = BufferFactory::create(gfx_context, vertex_data_size, BufferType::Vertex | BufferType::TransferDestination, MemoryUsageType::OnlyGPU);

		Renderer::get()->graphics_command_queue()->submit_immediate(gfx_context, [this, gfx_context, staging_buffer, vertex_data_size](void* command_buffer)
		{
			vertex_buffer->copy_buffer(gfx_context, command_buffer, staging_buffer, vertex_data_size);
		});

		staging_buffer->destroy(gfx_context);
		GlobalAssetPools<Buffer>::get()->deallocate(staging_buffer);
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

	Sunset::Mesh* MeshFactory::create_triangle(class GraphicsContext* const gfx_context)
	{
		static Mesh* mesh = GlobalAssetPools<Mesh>::get()->allocate();

		if (mesh->vertices.size() == 0)
		{
			mesh->vertices.resize(3);

			mesh->vertices[0].position = { 1.0f, 1.0f, 0.0f };
			mesh->vertices[1].position = { -1.0f, 1.0f, 0.0f };
			mesh->vertices[2].position = { 0.0f, -1.0f, 0.0f };

			mesh->vertices[0].color = { 0.0f, 1.0f, 0.0f };
			mesh->vertices[1].color = { 0.0f, 1.0f, 0.0f };
			mesh->vertices[2].color = { 0.0f, 1.0f, 0.0f };

			mesh->upload(gfx_context);
		}

		return mesh;
	}

	Sunset::Mesh* MeshFactory::load(class GraphicsContext* const gfx_context, const char* path)
	{
		const MeshID mesh_id = MeshCache::get()->fetch_or_add(path, gfx_context);
		Mesh* const mesh = MeshCache::get()->fetch(mesh_id);

		if (mesh->vertex_buffer == nullptr)
		{
			SerializedAsset asset;
			if (!deserialize_asset(path, asset))
			{
				// TODO: Need some custom logging
				return nullptr;
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

			mesh->upload(gfx_context);
		}

		return mesh;
	}

	void MeshCache::initialize()
	{
	}

	void MeshCache::update()
	{
	}

	Sunset::MeshID MeshCache::fetch_or_add(const char* file_path, class GraphicsContext* const gfx_context /*= nullptr*/)
	{
		MeshID id = std::hash<std::string>{}(file_path);
		if (cache.find(id) == cache.end())
		{
			Mesh* const new_mesh = GlobalAssetPools<Mesh>::get()->allocate();
			gfx_context->add_resource_deletion_execution([new_mesh, gfx_context]()
			{
				new_mesh->destroy(gfx_context);
				GlobalAssetPools<Mesh>::get()->deallocate(new_mesh);
			});
			cache.insert({ id, new_mesh });
		}
		return id;
	}

	Sunset::Mesh* MeshCache::fetch(MeshID id)
	{
		assert(cache.find(id) != cache.end());
		return cache[id];
	}

	void MeshCache::remove(MeshID id)
	{
		cache.erase(id);
	}

	void MeshCache::destroy(class GraphicsContext* const gfx_context)
	{
		for (const std::pair<size_t, Mesh*>& pair : cache)
		{
			pair.second->destroy(gfx_context);
		}
		cache.clear();
	}
}
