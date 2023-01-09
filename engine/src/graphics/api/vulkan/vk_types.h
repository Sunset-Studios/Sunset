﻿#pragma once

#include <iostream>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <minimal.h>
#include <descriptor_types.h>
#include <pipeline_types.h>
#include <image_types.h>

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