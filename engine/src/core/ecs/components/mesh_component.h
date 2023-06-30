#pragma once

#include <resource_types.h>
#include <pipeline_types.h>
#include <graphics/resource/material.h>
#include <glm/glm.hpp>

namespace Sunset
{
	struct MeshComponent
	{
		MeshID mesh;
		uint32_t section_count{ 0 };
		MaterialID materials[MAX_MESH_MATERIALS];
		ResourceStateID resource_states[MAX_MESH_RESOURCE_STATES];
	};

	void set_mesh(MeshComponent* mesh_comp, MeshID mesh);
	void set_material(MeshComponent* mesh_comp, MaterialID material, uint32_t section = 0);

	size_t mesh_vertex_count(MeshComponent* mesh_comp);
	size_t mesh_index_count(MeshComponent* mesh_comp, uint32_t section = 0);
	BufferID mesh_vertex_buffer(MeshComponent* mesh_comp);
	BufferID mesh_index_buffer(MeshComponent* mesh_comp, uint32_t section = 0);
	Bounds mesh_local_bounds(MeshComponent* mesh_comp);
	Bounds transform_mesh_bounds(MeshComponent* mesh_comp, glm::mat4 transform);
}
