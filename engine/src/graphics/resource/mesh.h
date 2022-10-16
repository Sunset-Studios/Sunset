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
		static Mesh* create_triangle(class GraphicsContext* const gfx_context);
	};
}
