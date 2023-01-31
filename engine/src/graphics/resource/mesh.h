#pragma once

#include <common.h>
#include <graphics/resource/resource_cache.h>

namespace Sunset
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 uv;

		static PipelineVertexInputDescription get_description();
	};

	struct Mesh
	{
		Mesh() = default;
		~Mesh() = default;

		Identity name;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		class Buffer* vertex_buffer;
		class Buffer* index_buffer;

		void upload(class GraphicsContext* const gfx_context);
		void destroy(class GraphicsContext* const gfx_context);
	};

	class MeshFactory
	{
	public:
		static MeshID create_triangle(class GraphicsContext* const gfx_context);
		static MeshID load(class GraphicsContext* const gfx_context, const char* path);
	};

	DEFINE_RESOURCE_CACHE(MeshCache, MeshID, Mesh);
}
