#pragma once

#include <common.h>
#include <utility/pattern/singleton.h>

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

	class MeshCache : public Singleton<MeshCache>
	{
		friend class Singleton;

	public:
		void initialize();
		void update();

		MeshID fetch_or_add(const char* file_path, class GraphicsContext* const gfx_context = nullptr);
		Mesh* fetch(MeshID id);
		void remove(MeshID id);
		void destroy(class GraphicsContext* const gfx_context);

		size_t size() const
		{
			return cache.size();
		}

	protected:
		std::unordered_map<MeshID, Mesh*> cache;

	private:
		MeshCache() = default;
	};
}
