#include <graphics/resource/mesh.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>
#include <graphics/asset_pool.h>
#include <tiny_obj_loader.h>

namespace Sunset
{
	void Mesh::upload(GraphicsContext* const gfx_context)
	{
		const size_t vertex_data_size = vertices.size() * sizeof(Vertex);

		if (vertex_buffer == nullptr)
		{
			vertex_buffer = BufferFactory::create(gfx_context, vertex_data_size, BufferType::Vertex);
		}

		vertex_buffer->copy_from(gfx_context, vertices.data(), vertex_data_size);
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

	Sunset::Mesh* MeshFactory::load_obj(class GraphicsContext* const gfx_context, const char* path)
	{
		const MeshID mesh_id = MeshCache::get()->fetch_or_add(path, gfx_context);
		Mesh* const mesh = MeshCache::get()->fetch(mesh_id);

		if (mesh->vertex_buffer == nullptr)
		{
			tinyobj::attrib_t attrib;
			std::vector<tinyobj::shape_t> shapes;
			std::vector<tinyobj::material_t> materials;

			std::string warn;
			std::string err;

			tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path, nullptr);

			if (!warn.empty())
			{
				// TODO: Need some custom logging
			}

			if (!err.empty())
			{
				// TODO: Need some custom logging
				return nullptr;
			}

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

						Vertex new_vertex;

						new_vertex.position.x = vx;
						new_vertex.position.y = vy;
						new_vertex.position.z = vz;

						new_vertex.normal.x = nx;
						new_vertex.normal.y = ny;
						new_vertex.normal.z = nz;

						new_vertex.color = new_vertex.normal;

						mesh->vertices.push_back(new_vertex);
					}
					index_offset += face_vertices;
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
			gfx_context->add_resource_deletion_execution([new_mesh, gfx_context]() { new_mesh->destroy(gfx_context); });
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
