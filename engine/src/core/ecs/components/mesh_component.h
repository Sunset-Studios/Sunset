#pragma once

#include <resource_types.h>
#include <pipeline_types.h>
#include <graphics/resource/material.h>
#include <glm/glm.hpp>

namespace Sunset
{
	struct MeshAdditionalData
	{
		glm::vec4 user_data;
	};

	struct MeshComponent
	{
		MeshID mesh;
		ResourceStateID resource_state;
		MaterialID material;
		MeshAdditionalData additional_data;
	};

	void set_mesh(MeshComponent* mesh_comp, MeshID mesh);
	void set_material(MeshComponent* mesh_comp, const Material& material);

	size_t mesh_vertex_count(MeshComponent* mesh_comp);
	size_t mesh_index_count(MeshComponent* mesh_comp);
	class Buffer* mesh_vertex_buffer(MeshComponent* mesh_comp);
	class Buffer* mesh_index_buffer(MeshComponent* mesh_comp);
}
