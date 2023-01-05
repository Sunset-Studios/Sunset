#pragma once

#define ENGINE_NAME "Sunset"

#include <glm/glm.hpp>

#include <graphics/viewport.h>

#include <memory>
#include <vector>
#include <array>
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
		Uint4x32,
		FloatDepth32,
		SRGB8x4
	};

	enum class MemoryUsageType : int32_t
	{
		OnlyCPU,
		OnlyGPU,
		GPUToCPU,
		CPUToGPU
	};

	enum class BufferType : int32_t
	{
		Generic = 0x00000000,
		Vertex = 0x00000001,
		UniformBuffer = 0x00000002,
		StorageBuffer = 0x00000004,
		TransferSource = 0x00000008,
		TransferDestination = 0x00000020
	};

	inline BufferType operator|(BufferType lhs, BufferType rhs)
	{
		return static_cast<BufferType>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
	}

	inline BufferType operator&(BufferType lhs, BufferType rhs)
	{
		return static_cast<BufferType>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
	}

	inline BufferType& operator|=(BufferType& lhs, BufferType rhs)
	{
		return lhs = lhs | rhs;
	}

	inline BufferType& operator&=(BufferType& lhs, BufferType rhs)
	{
		return lhs = lhs & rhs;
	}

	enum class ImageFlags : int32_t
	{
		None = 0x00000000,
		Color = 0x00000001,
		Depth = 0x00000002,
		Stencil = 0x00000004,
		TransferSrc = 0x00000008,
		TransferDst = 0x00000010,
		Image2D = 0x00000020,
		Image3D = 0x00000040,
		Sampled = 0x00000080
	};

	inline ImageFlags operator|(ImageFlags lhs, ImageFlags rhs)
	{
		return static_cast<ImageFlags>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
	}

	inline ImageFlags operator&(ImageFlags lhs, ImageFlags rhs)
	{
		return static_cast<ImageFlags>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
	}

	inline ImageFlags& operator|=(ImageFlags& lhs, ImageFlags rhs)
	{
		return lhs = lhs | rhs;
	}

	inline ImageFlags& operator&=(ImageFlags& lhs, ImageFlags rhs)
	{
		return lhs = lhs & rhs;
	}

	enum class CompareOperation : int16_t
	{
		Always,
		Equal,
		NotEqual,
		Less,
		LessOrEqual,
		Greater,
		GreaterOrEqual
	};

	using ObjectID = uint32_t;
	using ImageID = size_t;
	using MeshID = size_t;

	#define SECONDS_TIME std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() * 0.001
	#define WORLD_UP glm::vec3(0.0f, 1.0f, 0.0f)
	#define WORLD_FORWARD glm::vec3(0.0f, 0.0f, -1.0f)

	#define MAX_BUFFERED_FRAMES 2
}