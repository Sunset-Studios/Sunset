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
		mesh_comp->section_count = CACHE_FETCH(Mesh, mesh)->sections.size();
	}

	void set_material(MeshComponent* mesh_comp, MaterialID material, int32_t section)
	{
		assert(mesh_comp != nullptr && "Cannot set material on null mesh component");
		if (section >= 0)
		{
			mesh_comp->materials[section] = material;
		}
		else
		{
			mesh_comp->materials.fill(material);
		}
	}

	void set_custom_bounds_scale(MeshComponent* mesh_comp, float bounds_scale)
	{
		assert(mesh_comp != nullptr && "Cannot set bounds scale on null mesh component");
		mesh_comp->custom_bounds_scale = bounds_scale;
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

	size_t mesh_index_count(MeshComponent* mesh_comp, uint32_t section)
	{
		assert(mesh_comp != nullptr && "Cannot get index count via null mesh component");
		Mesh* const mesh = CACHE_FETCH(Mesh, mesh_comp->mesh);
		if (mesh != nullptr)
		{
			assert(section < mesh->sections.size() && "Out of bounds access on mesh sections array");
			return mesh->sections[section].indices.size();
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

	BufferID mesh_index_buffer(MeshComponent* mesh_comp, uint32_t section)
	{
		assert(mesh_comp != nullptr && "Cannot get index buffer via null mesh component");
		Mesh* const mesh = CACHE_FETCH(Mesh, mesh_comp->mesh);
		if (mesh != nullptr)
		{
			assert(section < mesh->sections.size() && "Out of bounds access on mesh sections array");
			return mesh->sections[section].index_buffer;
		}
		return 0;
	}

	Sunset::Bounds mesh_local_bounds(MeshComponent* mesh_comp)
	{
		assert(mesh_comp != nullptr && "Cannot get mesh bounds via null mesh component");
		Mesh* const mesh = CACHE_FETCH(Mesh, mesh_comp->mesh);
		if (mesh != nullptr)
		{
			return mesh->local_bounds;
		}
		return Bounds();
	}

	Sunset::Bounds transform_mesh_bounds(MeshComponent* mesh_comp, glm::mat4 transform)
	{
		assert(mesh_comp != nullptr && "Cannot calculate mesh bounds via null mesh component");
		Mesh* const mesh = CACHE_FETCH(Mesh, mesh_comp->mesh);
		if (mesh != nullptr)
		{
			return calculate_mesh_bounds(mesh, transform);
		}
		return Bounds();
	}
}
