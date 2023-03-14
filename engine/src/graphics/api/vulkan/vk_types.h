#pragma once

#include <iostream>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <minimal.h>
#include <descriptor_types.h>
#include <pipeline_types.h>
#include <image_types.h>
#include <renderer_types.h>

namespace Sunset
{
	struct VulkanGPUIndirectObject
	{
		VkDrawIndexedIndirectCommand indirect_command;
	};
}

inline void VK_CHECK(VkResult result)
{
	if (result)
	{
		std::cout << "Vulkan error detected: " << result << std::endl;
		abort();
	}
}

inline VkFormat VK_FROM_SUNSET_FORMAT(Sunset::Format format)
{
	switch (format)
	{
		case Sunset::Format::Undefined:
			return VK_FORMAT_UNDEFINED;

		case Sunset::Format::Int8:
			return VK_FORMAT_R8_SINT;
		case Sunset::Format::Uint8:
			return VK_FORMAT_R8_UINT;
		case Sunset::Format::Float16:
			return VK_FORMAT_R16_SFLOAT;
		case Sunset::Format::Int16:
			return VK_FORMAT_R16_SINT;
		case Sunset::Format::Uint16:
			return VK_FORMAT_R16_UINT;
		case Sunset::Format::Float32:
			return VK_FORMAT_R32_SFLOAT;
		case Sunset::Format::Int32:
			return VK_FORMAT_R32_SINT;
		case Sunset::Format::Uint32:
			return VK_FORMAT_R32_UINT;

		case Sunset::Format::Int2x8:
			return VK_FORMAT_R8G8_SINT;
		case Sunset::Format::Uint2x8:
			return VK_FORMAT_R8G8_UINT;
		case Sunset::Format::Float2x16:
			return VK_FORMAT_R16G16_SFLOAT;
		case Sunset::Format::Int2x16:
			return VK_FORMAT_R16G16_SINT;
		case Sunset::Format::Uint2x16:
			return VK_FORMAT_R16G16_UINT;
		case Sunset::Format::Float2x32:
			return VK_FORMAT_R32G32_SFLOAT;
		case Sunset::Format::Int2x32:
			return VK_FORMAT_R32G32_SINT;
		case Sunset::Format::Uint2x32:
			return VK_FORMAT_R32G32_UINT;

		case Sunset::Format::Int3x8:
			return VK_FORMAT_R8G8B8_SINT;
		case Sunset::Format::Uint3x8:
			return VK_FORMAT_R8G8B8_UINT;
		case Sunset::Format::Float3x16:
			return VK_FORMAT_R16G16B16_SFLOAT;
		case Sunset::Format::Int3x16:
			return VK_FORMAT_R16G16B16_SINT;
		case Sunset::Format::Uint3x16:
			return VK_FORMAT_R16G16B16_UINT;
		case Sunset::Format::Float3x32:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case Sunset::Format::Int3x32:
			return VK_FORMAT_R32G32B32_SINT;
		case Sunset::Format::Uint3x32:
			return VK_FORMAT_R32G32B32_UINT;

		case Sunset::Format::Int4x8:
			return VK_FORMAT_R8G8B8A8_SINT;
		case Sunset::Format::Uint4x8:
			return VK_FORMAT_R8G8B8A8_UINT;
		case Sunset::Format::Float4x16:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case Sunset::Format::Int4x16:
			return VK_FORMAT_R16G16B16A16_SINT;
		case Sunset::Format::Uint4x16:
			return VK_FORMAT_R16G16B16A16_UINT;
		case Sunset::Format::Float4x32:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case Sunset::Format::Int4x32:
			return VK_FORMAT_R32G32B32A32_SINT;
		case Sunset::Format::Uint4x32:
			return VK_FORMAT_R32G32B32A32_UINT;

		case Sunset::Format::FloatDepth32:
			return VK_FORMAT_D32_SFLOAT;

		case Sunset::Format::SRGB8x4:
			return VK_FORMAT_R8G8B8A8_SRGB;

		case Sunset::Format::UNorm4x8:
			return VK_FORMAT_R8G8B8A8_UNORM;

		default:
			return VK_FORMAT_UNDEFINED;
	}
}

inline Sunset::Format SUNSET_FROM_VK_FORMAT(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_UNDEFINED:
		return Sunset::Format::Undefined;

	case VK_FORMAT_R8_SINT:
		return Sunset::Format::Int8;
	case VK_FORMAT_R8_UINT:
		return Sunset::Format::Uint8;
	case VK_FORMAT_R16_SFLOAT:
		return Sunset::Format::Float16;
	case VK_FORMAT_R16_SINT:
		return Sunset::Format::Int16;
	case VK_FORMAT_R16_UINT:
		return Sunset::Format::Uint16;
	case VK_FORMAT_R32_SFLOAT:
		return Sunset::Format::Float32;
	case VK_FORMAT_R32_SINT:
		return Sunset::Format::Int32;
	case VK_FORMAT_R32_UINT:
		return Sunset::Format::Uint32;

	case VK_FORMAT_R8G8_SINT:
		return Sunset::Format::Int2x8;
	case VK_FORMAT_R8G8_UINT:
		return Sunset::Format::Uint2x8;
	case VK_FORMAT_R16G16_SFLOAT:
		return Sunset::Format::Float2x16;
	case VK_FORMAT_R16G16_SINT:
		return Sunset::Format::Int2x16;
	case VK_FORMAT_R16G16_UINT:
		return Sunset::Format::Uint2x16;
	case VK_FORMAT_R32G32_SFLOAT:
		return Sunset::Format::Float2x32;
	case VK_FORMAT_R32G32_SINT:
		return Sunset::Format::Int2x32;
	case VK_FORMAT_R32G32_UINT:
		return Sunset::Format::Uint2x32;

	case VK_FORMAT_R8G8B8_SINT:
		return Sunset::Format::Int3x8;
	case VK_FORMAT_R8G8B8_UINT:
		return Sunset::Format::Uint3x8;
	case VK_FORMAT_R16G16B16_SFLOAT:
		return Sunset::Format::Float3x16;
	case VK_FORMAT_R16G16B16_SINT:
		return Sunset::Format::Int3x16;
	case VK_FORMAT_R16G16B16_UINT:
		return Sunset::Format::Uint3x16;
	case VK_FORMAT_R32G32B32_SFLOAT:
		return Sunset::Format::Float3x32;
	case VK_FORMAT_R32G32B32_SINT:
		return Sunset::Format::Int3x32;
	case VK_FORMAT_R32G32B32_UINT:
		return Sunset::Format::Uint3x32;

	case VK_FORMAT_R8G8B8A8_SINT:
		return Sunset::Format::Int4x8;
	case VK_FORMAT_R8G8B8A8_UINT:
		return Sunset::Format::Uint4x8;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return Sunset::Format::Float4x16;
	case VK_FORMAT_R16G16B16A16_SINT:
		return Sunset::Format::Int4x16;
	case VK_FORMAT_R16G16B16A16_UINT:
		return Sunset::Format::Uint4x16;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		return Sunset::Format::Float4x32;
	case VK_FORMAT_R32G32B32A32_SINT:
		return Sunset::Format::Int4x32;
	case VK_FORMAT_R32G32B32A32_UINT:
		return Sunset::Format::Uint4x32;

	case VK_FORMAT_D32_SFLOAT:
		return Sunset::Format::FloatDepth32;

	case VK_FORMAT_B8G8R8A8_SRGB:
	case VK_FORMAT_R8G8B8A8_SRGB:
		return Sunset::Format::SRGB8x4;

	case VK_FORMAT_R8G8B8A8_UNORM:
		return Sunset::Format::UNorm4x8;

	default:
		return Sunset::Format::Undefined;
	}
}

inline VkImageType VK_FROM_SUNSET_IMAGE_TYPE(Sunset::ImageFlags image_type)
{
	if (static_cast<int32_t>(image_type & Sunset::ImageFlags::Image2D) > 0)
	{
		return VK_IMAGE_TYPE_2D;
	}
	else if (static_cast<int32_t>(image_type & Sunset::ImageFlags::Image3D) > 0)
	{
		return VK_IMAGE_TYPE_3D;
	}
	return VK_IMAGE_TYPE_2D;
}

inline VkImageViewType VK_FROM_SUNSET_IMAGE_VIEW_TYPE(Sunset::ImageFlags image_type)
{
	if (static_cast<int32_t>(image_type & Sunset::ImageFlags::Image2D) > 0)
	{
		return VK_IMAGE_VIEW_TYPE_2D;
	}
	else if (static_cast<int32_t>(image_type & Sunset::ImageFlags::Image3D) > 0)
	{
		return VK_IMAGE_VIEW_TYPE_3D;
	}
	return VK_IMAGE_VIEW_TYPE_2D;
}

inline VkDescriptorType VK_FROM_SUNSET_DESCRIPTOR_TYPE(Sunset::DescriptorType descriptor_type)
{
	switch (descriptor_type)
	{
	case Sunset::DescriptorType::UniformBuffer:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case Sunset::DescriptorType::DynamicUniformBuffer:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	case Sunset::DescriptorType::Image:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	case Sunset::DescriptorType::StorageBuffer:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	default:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	}
}

inline Sunset::DescriptorType SUNSET_FROM_VK_DESCRIPTOR_TYPE(VkDescriptorType descriptor_type)
{
	switch (descriptor_type)
	{
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		return Sunset::DescriptorType::UniformBuffer;
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		return Sunset::DescriptorType::DynamicUniformBuffer;
	case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		return Sunset::DescriptorType::Image;
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		return Sunset::DescriptorType::StorageBuffer;
	default:
		return Sunset::DescriptorType::StorageBuffer;
	}
}

inline VkShaderStageFlagBits VK_FROM_SUNSET_SHADER_STAGE_TYPE(Sunset::PipelineShaderStageType shader_stage_type)
{
	uint32_t sunset_shader_stages{ static_cast<uint32_t>(shader_stage_type) };
	uint32_t vk_shader_stages{ 0 };
	if (sunset_shader_stages & static_cast<uint32_t>(Sunset::PipelineShaderStageType::Vertex))
	{
		vk_shader_stages |= VK_SHADER_STAGE_VERTEX_BIT;
	}
	if (sunset_shader_stages & static_cast<uint32_t>(Sunset::PipelineShaderStageType::Fragment))
	{
		vk_shader_stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
	}
	if (sunset_shader_stages & static_cast<uint32_t>(Sunset::PipelineShaderStageType::Geometry))
	{
		vk_shader_stages |= VK_SHADER_STAGE_GEOMETRY_BIT;
	}
	if (sunset_shader_stages & static_cast<uint32_t>(Sunset::PipelineShaderStageType::Compute))
	{
		vk_shader_stages |= VK_SHADER_STAGE_COMPUTE_BIT;
	}
	return static_cast<VkShaderStageFlagBits>(vk_shader_stages);
}

inline VkPipelineBindPoint VK_FROM_SUNSET_PIPELINE_STATE_BIND_TYPE(Sunset::PipelineStateType state_type)
{
	switch (state_type)
	{
	case Sunset::PipelineStateType::Graphics:
		return VK_PIPELINE_BIND_POINT_GRAPHICS;
	case Sunset::PipelineStateType::Compute:
		return VK_PIPELINE_BIND_POINT_COMPUTE;
	default:
		return VK_PIPELINE_BIND_POINT_GRAPHICS;
	}
}

inline VkPipelineStageFlagBits VK_FROM_SUNSET_PIPELINE_STAGE_TYPE(Sunset::PipelineStageType pipeline_stage_type)
{
	uint32_t sunset_pipeline_stages{ static_cast<uint32_t>(pipeline_stage_type) };
	uint32_t vk_pipeline_stages{ 0 };
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::TopOfPipe))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::BottomOfPipe))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::Host))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_HOST_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::DrawIndirect))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::VertexInput))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::VertexShader))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::TessellationControl))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::TessellationEvaluation))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::GeometryShader))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::FragmentShader))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::EarlyFragmentTest))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::LateFragmentTest))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::ColorAttachmentOutput))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::ComputeShader))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::Transfer))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::AllGraphics))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	}
	if (sunset_pipeline_stages & static_cast<uint32_t>(Sunset::PipelineStageType::AllCommands))
	{
		vk_pipeline_stages |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	}
	return static_cast<VkPipelineStageFlagBits>(vk_pipeline_stages);
}

inline VkAccessFlagBits VK_FROM_SUNSET_ACCESS_FLAGS(Sunset::AccessFlags access_flags)
{
	uint32_t sunset_access_stages{ static_cast<uint32_t>(access_flags) };
	uint32_t vk_access_flags{ 0 };
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::IndirectCommandRead))
	{
		vk_access_flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::IndexRead))
	{
		vk_access_flags |= VK_ACCESS_INDEX_READ_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::VertexAttributeRead))
	{
		vk_access_flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::UniformRead))
	{
		vk_access_flags |= VK_ACCESS_UNIFORM_READ_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::InputAttachmentRead))
	{
		vk_access_flags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::ShaderRead))
	{
		vk_access_flags |= VK_ACCESS_SHADER_READ_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::ShaderWrite))
	{
		vk_access_flags |= VK_ACCESS_SHADER_WRITE_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::ColorAttachmentRead))
	{
		vk_access_flags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::ColorAttachmentWrite))
	{
		vk_access_flags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::DepthStencilAttachmentRead))
	{
		vk_access_flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::DepthStencilAttachmentWrite))
	{
		vk_access_flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::TransferRead))
	{
		vk_access_flags |= VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::TransferWrite))
	{
		vk_access_flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::HostRead))
	{
		vk_access_flags |= VK_ACCESS_HOST_READ_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::HostWrite))
	{
		vk_access_flags |= VK_ACCESS_HOST_WRITE_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::MemoryRead))
	{
		vk_access_flags |= VK_ACCESS_MEMORY_READ_BIT;
	}
	if (sunset_access_stages & static_cast<uint32_t>(Sunset::AccessFlags::MemoryWrite))
	{
		vk_access_flags |= VK_ACCESS_MEMORY_WRITE_BIT;
	}
	return static_cast<VkAccessFlagBits>(vk_access_flags);
}

inline VkBlendFactor VK_FROM_SUNSET_BLEND_FACTOR(Sunset::BlendFactor blend_factor)
{
	switch (blend_factor)
	{
	case Sunset::BlendFactor::Zero:
		return VK_BLEND_FACTOR_ZERO;
	case Sunset::BlendFactor::One:
		return VK_BLEND_FACTOR_ONE;
	case Sunset::BlendFactor::SourceColor:
		return VK_BLEND_FACTOR_SRC_COLOR;
	case Sunset::BlendFactor::OneMinusSourceColor:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
	case Sunset::BlendFactor::DestinationColor:
		return VK_BLEND_FACTOR_DST_COLOR;
	case Sunset::BlendFactor::OneMinusDestinationColor:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	case Sunset::BlendFactor::SourceAlpha:
		return VK_BLEND_FACTOR_SRC_ALPHA;
	case Sunset::BlendFactor::OneMinusSourceAlpha:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case Sunset::BlendFactor::DestinationAlpha:
		return VK_BLEND_FACTOR_DST_ALPHA;
	case Sunset::BlendFactor::OneMinusDestinationAlpha:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	default:
		return VK_BLEND_FACTOR_ZERO;
	}
}

inline VkBlendOp VK_FROM_SUNSET_BLEND_OP(Sunset::BlendOp blend_op)
{
	switch (blend_op)
	{
	case Sunset::BlendOp::Add:
		return VK_BLEND_OP_ADD;
	case Sunset::BlendOp::Subtract:
		return VK_BLEND_OP_SUBTRACT;
	case Sunset::BlendOp::ReverseSubtract:
		return VK_BLEND_OP_REVERSE_SUBTRACT;
	case Sunset::BlendOp::Min:
		return VK_BLEND_OP_MIN;
	case Sunset::BlendOp::Max:
		return VK_BLEND_OP_MAX;
	default:
		return VK_BLEND_OP_ADD;
	}
}

inline std::vector<VkDescriptorSetLayoutBinding> VK_FROM_SUNSET_DESCRIPTOR_BINDINGS(const std::vector<Sunset::DescriptorBinding>& bindings)
{
	std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
	vk_bindings.reserve(bindings.size());

	for (const Sunset::DescriptorBinding& binding : bindings)
	{
		VkDescriptorSetLayoutBinding new_vk_binding = {};
		new_vk_binding.binding = binding.slot;
		new_vk_binding.descriptorCount = binding.count;
		new_vk_binding.descriptorType = VK_FROM_SUNSET_DESCRIPTOR_TYPE(binding.type);
		new_vk_binding.stageFlags = VK_FROM_SUNSET_SHADER_STAGE_TYPE(binding.pipeline_stages);
		vk_bindings.push_back(new_vk_binding);
	}

	return vk_bindings;
}

inline VmaMemoryUsage VK_FROM_SUNSET_MEMORY_USAGE(Sunset::MemoryUsageType type)
{
	switch (type)
	{
	case Sunset::MemoryUsageType::OnlyCPU:
		return VMA_MEMORY_USAGE_CPU_ONLY;
	case Sunset::MemoryUsageType::OnlyGPU:
		return VMA_MEMORY_USAGE_GPU_ONLY;
	case Sunset::MemoryUsageType::CPUToGPU:
		return VMA_MEMORY_USAGE_CPU_TO_GPU;
	case Sunset::MemoryUsageType::GPUToCPU:
		return VMA_MEMORY_USAGE_GPU_TO_CPU;
	default:
		return VMA_MEMORY_USAGE_UNKNOWN;
	}
}

inline VkFilter VK_FROM_SUNSET_IMAGE_FILTER(Sunset::ImageFilter filter)
{
	switch (filter)
	{
	case Sunset::ImageFilter::Nearest:
		return VK_FILTER_NEAREST;
	case Sunset::ImageFilter::Linear:
		return VK_FILTER_LINEAR;
	case Sunset::ImageFilter::Cubic:
		return VK_FILTER_CUBIC_EXT;
	default:
		return VK_FILTER_LINEAR;
	}
}

inline VkSamplerAddressMode VK_FROM_SUNSET_SAMPLER_ADDRESS_MODE(Sunset::SamplerAddressMode mode)
{
	switch (mode)
	{
	case Sunset::SamplerAddressMode::Mirrored:
		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case Sunset::SamplerAddressMode::Repeat:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case Sunset::SamplerAddressMode::EdgeClamp:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case Sunset::SamplerAddressMode::BorderClamp:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	case Sunset::SamplerAddressMode::MirroredEdgeClamp:
		return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
	default:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}
}