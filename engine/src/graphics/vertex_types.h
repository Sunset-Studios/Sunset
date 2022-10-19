#pragma once

#include <minimal.h>

namespace Sunset
{
	struct VertexBinding
	{
		int16_t index{ 0 };
		size_t stride{ 0 };

		bool operator==(const VertexBinding& other) const
		{
			return index == other.index && stride == other.stride;
		}
	};

	struct VertexAttribute
	{
		int16_t binding{ 0 };
		int16_t index{ 0 };
		Format format{ Format::Undefined };
		size_t data_offset{ 0 };

		bool operator==(const VertexAttribute& other) const
		{
			return binding == other.binding
				&& index == other.index
				&& format == other.format
				&& data_offset == other.data_offset;
		}
	};
}