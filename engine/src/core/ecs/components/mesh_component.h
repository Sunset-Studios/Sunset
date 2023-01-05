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
		struct Mesh* mesh;
		ResourceStateID resource_state;
		Material material;
		MeshAdditionalData additional_data;
	};

	void set_mesh(MeshComponent* mesh_comp, struct Mesh* const mesh);
	void set_material(MeshComponent* mesh_comp, const Material& material);

	size_t mesh_vertex_count(MeshComponent* mesh_comp);
}
