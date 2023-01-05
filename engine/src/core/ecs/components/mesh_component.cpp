#include <core/ecs/components/mesh_component.h>
#include <graphics/resource/mesh.h>
#include <graphics/resource_state.h>
#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>
#include <minimal.h>

namespace Sunset
{
	void set_mesh(MeshComponent* mesh_comp, Mesh* const mesh)
	{
		assert(mesh_comp != nullptr && "Cannot set mesh on null mesh component");
		mesh_comp->mesh = mesh;
	}

	void set_material(MeshComponent* mesh_comp, const Material& material)
	{
		assert(mesh_comp != nullptr && "Cannot set material on null mesh component");
		mesh_comp->material = material;
	}

	size_t mesh_vertex_count(MeshComponent* mesh_comp)
	{
		assert(mesh_comp != nullptr && "Cannot get vertex count via null mesh component");
		if (mesh_comp->mesh != nullptr)
		{
			return mesh_comp->mesh->vertices.size();
		}
		return 0;
	}
}
