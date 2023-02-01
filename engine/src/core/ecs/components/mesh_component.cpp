#include <core/ecs/components/mesh_component.h>
#include <graphics/resource/mesh.h>
#include <graphics/resource_state.h>
#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <minimal.h>

namespace Sunset
{
	void set_mesh(MeshComponent* mesh_comp, MeshID mesh)
	{
		assert(mesh_comp != nullptr && "Cannot set mesh on null mesh component");
		mesh_comp->mesh = mesh;
	}

	void set_material(MeshComponent* mesh_comp, const Material& material)
	{
		assert(mesh_comp != nullptr && "Cannot set material on null mesh component");
		Identity cache_id = std::hash<Material>{}(material);
		mesh_comp->material = MaterialCache::get()->fetch_or_add(cache_id, Renderer::get()->context());
	}

	size_t mesh_vertex_count(MeshComponent* mesh_comp)
	{
		assert(mesh_comp != nullptr && "Cannot get vertex count via null mesh component");
		Mesh* const mesh = CACHE_FETCH(Mesh, mesh_comp->mesh);
		if (mesh != nullptr)
		{
			return mesh->vertices.size();
		}
		return 0;
	}

	size_t mesh_index_count(MeshComponent* mesh_comp)
	{
		assert(mesh_comp != nullptr && "Cannot get index count via null mesh component");
		Mesh* const mesh = CACHE_FETCH(Mesh, mesh_comp->mesh);
		if (mesh != nullptr)
		{
			return mesh->indices.size();
		}
		return 0;
	}

	BufferID mesh_vertex_buffer(MeshComponent* mesh_comp)
	{
		assert(mesh_comp != nullptr && "Cannot get vertex buffer via null mesh component");
		Mesh* const mesh = CACHE_FETCH(Mesh, mesh_comp->mesh);
		if (mesh != nullptr)
		{
			return mesh->vertex_buffer;
		}
		return 0;
	}

	BufferID mesh_index_buffer(MeshComponent* mesh_comp)
	{
		assert(mesh_comp != nullptr && "Cannot get index buffer via null mesh component");
		Mesh* const mesh = CACHE_FETCH(Mesh, mesh_comp->mesh);
		if (mesh != nullptr)
		{
			return mesh->index_buffer;
		}
		return 0;
	}
}
