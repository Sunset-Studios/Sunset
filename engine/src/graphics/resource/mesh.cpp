#include <graphics/resource/mesh.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>

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
}
