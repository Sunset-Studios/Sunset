#pragma once

#include <common.h>

namespace Sunset
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;

		static PipelineVertexInputDescription get_description();
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		class Buffer* vertex_buffer;

		void upload(class GraphicsContext* const gfx_context);
	};

	class MeshFactory
	{
	public:
		static Mesh* create_triangle(class GraphicsContext* const gfx_context)
		{
			Mesh* mesh = new Mesh;

			mesh->vertices.resize(3);

			mesh->vertices[0].position = { 1.0f, 1.0f, 0.0f };
			mesh->vertices[1].position = { -1.0f, 1.0f, 0.0f };
			mesh->vertices[2].position = { 0.0f, -1.0f, 0.0f };

			mesh->vertices[0].color = { 0.0f, 1.0f, 0.0f };
			mesh->vertices[1].color = { 0.0f, 1.0f, 0.0f };
			mesh->vertices[2].color = { 0.0f, 1.0f, 0.0f };

			mesh->upload(gfx_context);

			return mesh;
		}
	};
}
