#pragma once

#include <stdint.h>

#include <graphics/pipeline_types.h>
#include <utility/maths.h>

namespace Sunset
{
	using DescriptorLayoutID = size_t;
	using DescriptorSetID = size_t;

	enum class DescriptorType : int16_t
	{
		UniformBuffer,
		Image,
		DynamicUniformBuffer
	};

	enum class DescriptorSetType : int16_t
	{
		Global = 0,
		Material,
		Object,
		MaxSets
	};

	constexpr uint32_t MAX_BOUND_DESCRIPTOR_SETS = static_cast<uint32_t>(DescriptorSetType::MaxSets);

	struct DescriptorData
	{
		class DescriptorSet* descriptor_set;
		class DescriptorLayout* descriptor_layout;
		std::vector<uint32_t> dynamic_buffer_offsets;
	};

	struct DescriptorBuildData
	{
		uint16_t binding{ 0 };
		union
		{
			class Buffer* buffer{ nullptr };
			class Image* image;
		};
		uint32_t buffer_offset{ 0 };
		size_t buffer_range{ 0 };
		DescriptorType type;
		PipelineShaderStageType shader_stages;
	};

	struct DescriptorWrite
	{
		uint16_t slot{ 0 };
		uint32_t count{ 0 };
		DescriptorType type;
		void* buffer{ nullptr };
		size_t buffer_size{ 0 };
		size_t buffer_range{ 0 };
		class DescriptorSet* set{ nullptr };
	};

	struct DescriptorBinding
	{
		uint16_t slot{ 0 };
		uint32_t count{ 0 };
		DescriptorType type;
		PipelineShaderStageType pipeline_stages;

		bool operator==(const DescriptorBinding& other) const
		{
			return slot == other.slot && count == other.count && type == other.type && pipeline_stages == other.pipeline_stages;
		}
	};
}


#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4267)

template<>
struct std::hash<Sunset::DescriptorBinding>
{
	std::size_t operator()(const Sunset::DescriptorBinding& binding) const
	{
		std::size_t hash = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(binding.slot), binding.count);
		hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(binding.type));
		hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(binding.pipeline_stages));
		return hash;
	}
};

#pragma warning( pop ) 