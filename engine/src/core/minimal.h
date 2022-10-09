#pragma once

#define ENGINE_NAME "Sunset"

#include <glm/glm.hpp>

#include <graphics/viewport.h>

#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <typeinfo>
#include <cassert>

namespace Sunset
{
	enum class Format : int16_t
	{
		Undefined = 0,
		Int8,
		Uint8,
		Float16,
		Int16,
		Uint16,
		Float32,
		Int32,
		Uint32,
		Int2x8,
		Uint2x8,
		Float2x16,
		Int2x16,
		Uint2x16,
		Float2x32,
		Int2x32,
		Uint2x32,
		Int3x8,
		Uint3x8,
		Float3x16,
		Int3x16,
		Uint3x16,
		Float3x32,
		Int3x32,
		Uint3x32,
		Int4x8,
		Uint4x8,
		Float4x16,
		Int4x16,
		Uint4x16,
		Float4x32,
		Int4x32,
		Uint4x32
	};

	enum class BufferType : int16_t
	{
		Generic,
		Vertex
	};

	using ObjectID = uint32_t;

	#define SECONDS_TIME std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() * 0.001
}