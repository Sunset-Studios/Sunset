#pragma once

#include <minimal.h>

#include <string>
#include <vector>

namespace Sunset
{
	enum class CompressionMode : uint16_t
	{
		None = 0,
		LZ4
	};

	struct SerializedAsset
	{
		char type[4];
		int version;
		std::string metadata;
		std::vector<char> binary;
	};

	bool serialize_asset(const char* path, const SerializedAsset& asset);
	bool deserialize_asset(const char* path, SerializedAsset& out_asset);

	inline Format SUNSET_FORMAT_FROM_STRING(const char* format_str)
	{
		if (strcmp(format_str, "Int8") == 0)
		{
			return Format::Int8;
		}
		else if (strcmp(format_str, "Float16") == 0)
		{
			return Format::Float16;
		}
		else if (strcmp(format_str, "Int16") == 0)
		{
			return Format::Int16;
		}
		else if (strcmp(format_str, "Uint16") == 0)
		{
			return Format::Uint16;
		}
		else if (strcmp(format_str, "Float32") == 0)
		{
			return Format::Float32;
		}
		else if (strcmp(format_str, "Int32") == 0)
		{
			return Format::Int32;
		}
		else if (strcmp(format_str, "Uint32") == 0)
		{
			return Format::Uint32;
		}
		else if (strcmp(format_str, "Uint2x8") == 0)
		{
			return Format::Uint2x8;
		}
		else if (strcmp(format_str, "Float2x16") == 0)
		{
			return Format::Float2x16;
		}
		else if (strcmp(format_str, "Int2x16") == 0)
		{
			return Format::Int2x16;
		}
		else if (strcmp(format_str, "Uint2x16") == 0)
		{
			return Format::Uint2x16;
		}
		else if (strcmp(format_str, "Float2x32") == 0)
		{
			return Format::Float2x32;
		}
		else if (strcmp(format_str, "Int2x32") == 0)
		{
			return Format::Int2x32;
		}
		else if (strcmp(format_str, "Uint2x32") == 0)
		{
			return Format::Uint2x32;
		}
		else if (strcmp(format_str, "Int3x8") == 0)
		{
			return Format::Int3x8;
		}
		else if (strcmp(format_str, "Uint3x8") == 0)
		{
			return Format::Uint3x8;
		}
		else if (strcmp(format_str, "Float3x16") == 0)
		{
			return Format::Float3x16;
		}
		else if (strcmp(format_str, "Int3x16") == 0)
		{
			return Format::Int3x16;
		}
		else if (strcmp(format_str, "Uint3x16") == 0)
		{
			return Format::Uint3x16;
		}
		else if (strcmp(format_str, "Float3x32") == 0)
		{
			return Format::Float3x32;
		}
		else if (strcmp(format_str, "Int3x32") == 0)
		{
			return Format::Int3x32;
		}
		else if (strcmp(format_str, "Uint3x32") == 0)
		{
			return Format::Uint3x32;
		}
		else if (strcmp(format_str, "Int4x8") == 0)
		{
			return Format::Int4x8;
		}
		else if (strcmp(format_str, "Uint4x8") == 0)
		{
			return Format::Uint4x8;
		}
		else if (strcmp(format_str, "Float4x16") == 0)
		{
			return Format::Float4x16;
		}
		else if (strcmp(format_str, "Int4x16") == 0)
		{
			return Format::Int4x16;
		}
		else if (strcmp(format_str, "Uint4x16") == 0)
		{
			return Format::Uint4x16;
		}
		else if (strcmp(format_str, "Float4x32") == 0)
		{
			return Format::Float4x32;
		}
		else if (strcmp(format_str, "Int4x32") == 0)
		{
			return Format::Int4x32;
		}
		else if (strcmp(format_str, "Uint4x32") == 0)
		{
			return Format::Uint4x32;
		}
		else if (strcmp(format_str, "FloatDepth32") == 0)
		{
			return Format::FloatDepth32;
		}
		else if (strcmp(format_str, "SRGB8x4") == 0)
		{
			return Format::SRGB8x4;
		}
		else if (strcmp(format_str, "UNorm4x8") == 0)
		{
			return Format::UNorm4x8;
		}
		else
		{
			return Format::Undefined;
		}
	}
	inline const char* SUNSET_FORMAT_TO_STRING(Format format)
	{
		switch (format)
		{
		case Format::Int8:
			return "Int8";
		case Format::Uint8:
			return "Uint8";
		case Format::Float16:
			return "Float16";
		case Format::Int16:
			return "Int16";
		case Format::Uint16:
			return "Uint16";
		case Format::Float32:
			return "Float32";
		case Format::Int32:
			return "Int32";
		case Format::Uint32:
			return "Uint32";
		case Format::Int2x8:
			return "Int2x8";
		case Format::Uint2x8:
			return "Uint2x8";
		case Format::Float2x16:
			return "Float2x16";
		case Format::Int2x16:
			return "Int2x16";
		case Format::Uint2x16:
			return "Uint2x16";
		case Format::Float2x32:
			return "Float2x32";
		case Format::Int2x32:
			return "Int2x32";
		case Format::Uint2x32:
			return "Uint2x32";
		case Format::Int3x8:
			return "Int3x8";
		case Format::Uint3x8:
			return "Uint3x8";
		case Format::Float3x16:
			return "Float3x16";
		case Format::Int3x16:
			return "Int3x16";
		case Format::Uint3x16:
			return "Uint3x16";
		case Format::Float3x32:
			return "Float3x32";
		case Format::Int3x32:
			return "Int3x32";
		case Format::Uint3x32:
			return "Uint3x32";
		case Format::Int4x8:
			return "Int4x8";
		case Format::Uint4x8:
			return "Uint4x8";
		case Format::Float4x16:
			return "Float4x16";
		case Format::Int4x16:
			return "Int4x16";
		case Format::Uint4x16:
			return "Uint4x16";
		case Format::Float4x32:
			return "Float4x32";
		case Format::Int4x32:
			return "Int4x32";
		case Format::Uint4x32:
			return "Uint4x32";
		case Format::FloatDepth32:
			return "FloatDepth32";
		case Format::SRGB8x4:
			return "SRGB8x4";
		case Format::UNorm4x8:
			return "UNorm4x8";
		default:
			return "Undefined";
		}
	}

	inline CompressionMode SUNSET_COMPRESSION_MODE_FROM_STRING(const char* compression_str)
	{
		if (strcmp(compression_str, "LZ4") == 0)
		{
			return CompressionMode::LZ4;
		}
		else
		{
			return CompressionMode::None;
		}
	}
	inline const char* SUNSET_COMPRESSION_MODE_TO_STRING(CompressionMode compression_mode)
	{
		switch (compression_mode)
		{
		case CompressionMode::LZ4:
			return "LZ4";
		default:
			return "Undefined";
		}
	}
}
