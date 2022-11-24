#pragma once

#include <resource_types.h>
#include <pipeline_types.h>
#include <glm/glm.hpp>

namespace Sunset
{
	struct MeshUniformData
	{
		glm::mat4 transform_matrix;
		glm::vec4 user_data;
	};

	struct MeshComponent
	{
		struct Mesh* mesh;
		ResourceStateID resource_state;
		PipelineStateID pipeline_state;
		PipelineShaderPathList shaders;
		MeshUniformData uniform_data;
	};

	void set_mesh(MeshComponent* mesh_comp, struct Mesh* const mesh);
	void set_shaders(MeshComponent* mesh_comp, const PipelineShaderPathList& mesh_shaders);

	size_t mesh_vertex_count(MeshComponent* mesh_comp);
}
