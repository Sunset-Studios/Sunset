﻿#pragma once

#include <iostream>

#include <vulkan/vulkan.h>

#include <minimal.h>

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