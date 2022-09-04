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
}
