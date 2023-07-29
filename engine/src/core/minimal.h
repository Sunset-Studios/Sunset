#pragma once

#define ENGINE_NAME "Sunset"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <memory>
#include <vector>
#include <array>
#include <chrono>
#include <typeinfo>
#include <cassert>

#include <phmap.h>

#ifdef _DEBUG
#include <tracy/Tracy.hpp>
#endif

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
		SRGB8x4,
		UNorm4x8
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
		None = 0x00000000,
		Vertex = 0x00000001,
		Index = 0x00000002,
		UniformBuffer = 0x00000004,
		StorageBuffer = 0x00000008,
		TransferSource = 0x00000010,
		TransferDestination = 0x00000020,
		Indirect = 0x00000040,
		Transient = 0x00000080
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
		DepthStencil = 0x00000008,
		TransferSrc = 0x00000010,
		TransferDst = 0x00000020,
		Image2D = 0x00000040,
		Image2DArray = 0x00000080,
		Image3D = 0x00000100,
		Sampled = 0x00000200,
		Present = 0x00000400,
		Transient = 0x00000800,
		LocalLoad = 0x00001000,
		Storage = 0x00002000,
		Cube = 0x00004000,
		LinearTiling = 0x00004000
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

	enum class ImageLayout : int32_t
	{
		Undefined = 0,
		General,
		ColorAttachment,
		DepthStencilAttachment,
		DepthStencilReadOnly,
		ShaderReadOnly,
		TransferSource,
		TransferDestination,
		Preinitialized
	};

	enum class AccessFlags : int32_t
	{
		None = 0,
		IndirectCommandRead = 0x00000001,
		IndexRead = 0x00000002,
		VertexAttributeRead = 0x00000004,
		UniformRead = 0x00000008,
		InputAttachmentRead = 0x00000010,
		ShaderRead = 0x00000020,
		ShaderWrite = 0x00000040,
		ColorAttachmentRead = 0x00000080,
		ColorAttachmentWrite = 0x00000100,
		DepthStencilAttachmentRead = 0x00000200,
		DepthStencilAttachmentWrite = 0x00000400,
		TransferRead = 0x00000800,
		TransferWrite = 0x00001000,
		HostRead = 0x00002000,
		HostWrite = 0x00004000,
		MemoryRead = 0x00008000,
		MemoryWrite = 0x00010000
	};

	inline AccessFlags operator|(AccessFlags lhs, AccessFlags rhs)
	{
		return static_cast<AccessFlags>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
	}

	inline AccessFlags operator&(AccessFlags lhs, AccessFlags rhs)
	{
		return static_cast<AccessFlags>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
	}

	inline AccessFlags& operator|=(AccessFlags& lhs, AccessFlags rhs)
	{
		return lhs = lhs | rhs;
	}

	inline AccessFlags& operator&=(AccessFlags& lhs, AccessFlags rhs)
	{
		return lhs = lhs & rhs;
	}

	enum class CompareOperation : uint16_t
	{
		Always,
		Equal,
		NotEqual,
		Less,
		LessOrEqual,
		Greater,
		GreaterOrEqual
	};

	enum class ResourceType : uint16_t
	{
		Undefined = 0,
		Image,
		Buffer
	};

	using ObjectID = uint32_t;
	using ImageID = size_t;
	using MeshID = size_t;
	using MaterialID = size_t;
	using BufferID = size_t;
	using RenderPassID = size_t;
	using FramebufferID = size_t;
	using ShaderLayoutID = size_t;
	using ShaderID = size_t;

	#define SECONDS_TIME std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() * 0.001
	#define WORLD_UP glm::vec3(0.0f, 1.0f, 0.0f)
	#define WORLD_FORWARD glm::vec3(0.0f, 0.0f, -1.0f)
	#define WORLD_RIGHT glm::vec3(1.0f, 0.0f, 0.0f)
	#define PI 3.141592653f
	#define TWOPI 6.28318530718f

	constexpr uint16_t MAX_BUFFERED_FRAMES = 2;
	constexpr uint16_t MAX_MATERIALS = 16536;
	constexpr uint16_t MAX_MATERIAL_TEXTURES = 16;
	constexpr uint16_t MAX_MESH_RESOURCE_STATES = 32;
	constexpr uint16_t MAX_MESH_MATERIALS = MAX_MESH_RESOURCE_STATES;
	constexpr uint32_t MIN_ENTITIES = 8192;
	constexpr uint32_t MAX_SHADOW_CASCADES = 4;

	struct Bounds
	{
		glm::vec3 extents;
		glm::vec3 origin;
		float radius;
	};
}