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
		glm::vec3 tangent;
		glm::vec3 bitangent;

		static PipelineVertexInputDescription get_description();
	};

	struct Mesh
	{
		Mesh() = default;
		~Mesh() = default;

		Identity name;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		BufferID vertex_buffer;
		BufferID index_buffer;
		Bounds local_bounds;

		void destroy(class GraphicsContext* gfx_context) { }
	};

	void upload_mesh(class GraphicsContext* const gfx_context, Mesh* mesh);
	void destroy_mesh(class GraphicsContext* const gfx_context, Mesh* mesh);
	Bounds calculate_mesh_bounds(Mesh* mesh, const glm::mat4& transform);
	void update_mesh_tangent_bitangents(Mesh* mesh);

	Bounds get_mesh_bounds(MeshID mesh);

	class MeshFactory
	{
	public:
		static MeshID create_triangle(class GraphicsContext* const gfx_context);
		static MeshID create_quad(class GraphicsContext* const gfx_context);
		static MeshID create_sphere(class GraphicsContext* const gfx_context, const glm::ivec2& segments, float radius);
		static MeshID load(class GraphicsContext* const gfx_context, const char* path);
	};

	DEFINE_RESOURCE_CACHE(MeshCache, MeshID, Mesh);
}
