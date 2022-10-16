#include <graphics/resource/mesh.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>
#include <graphics/asset_pool.h>

namespace Sunset
{
	void Mesh::upload(GraphicsContext* const gfx_context)
	{
		const size_t vertex_data_size = vertices.size() * sizeof(Vertex);

		if (vertex_buffer == nullptr)
		{
			vertex_buffer = BufferFactory::create(gfx_context, vertex_data_size, BufferType::Vertex);
		}

		vertex_buffer->copy_from(gfx_context, vertices.data(), vertex_data_size);
	}

	Sunset::PipelineVertexInputDescription Vertex::get_description()
	{
		PipelineVertexInputDescription description;

		description.bindings.emplace_back(0, sizeof(Vertex));

		description.attributes.emplace_back(0, 0, Format::Float3x32, offsetof(Vertex, position));
		description.attributes.emplace_back(0, 1, Format::Float3x32, offsetof(Vertex, normal));
		description.attributes.emplace_back(0, 2, Format::Float3x32, offsetof(Vertex, color));

		return description;
	}

	Sunset::Mesh* MeshFactory::create_triangle(class GraphicsContext* const gfx_context)
	{
		Mesh* mesh = GlobalAssetPools<Mesh>::get()->allocate();

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

}
