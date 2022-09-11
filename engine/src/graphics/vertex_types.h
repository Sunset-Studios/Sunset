#pragma once

#include <minimal.h>

namespace Sunset
{
	struct VertexBinding
	{
		int16_t index{ 0 };
		size_t stride{ 0 };
	};

	struct VertexAttribute
	{
		int16_t binding{ 0 };
		int16_t index{ 0 };
		Format format{ Format::Undefined };
		size_t data_offset{ 0 };
	};
}