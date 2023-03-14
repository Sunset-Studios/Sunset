#pragma once

#include <stdint.h>

#include <graphics/pipeline_types.h>
#include <utility/maths.h>

namespace Sunset
{
	constexpr uint32_t ImageBindTableSlot = 2;

	using DescriptorLayoutID = size_t;
	using DescriptorSetID = size_t;

	enum class DescriptorType : int16_t
	{
		UniformBuffer,
		Image,
		DynamicUniformBuffer,
		StorageBuffer
	};

	enum class DescriptorSetType : uint16_t
	{
		Global = 0,
		Pass,
		Material,
		Object,
		MaxSets
	};

	constexpr uint32_t MAX_BOUND_DESCRIPTOR_SETS = static_cast<uint32_t>(DescriptorSetType::MaxSets);

	struct DescriptorData
	{
		class DescriptorSet* descriptor_set;
		DescriptorLayoutID descriptor_layout;
		ShaderLayoutID pipeline_layout;
	};

	struct DescriptorDataList
	{
		std::vector<class DescriptorSet*> descriptor_sets;
		std::vector<DescriptorLayoutID> descriptor_layouts;
		ShaderLayoutID pipeline_layout;
	};

	struct DescriptorBuildData
	{
		uint16_t binding{ 0 };
		union
		{
			BufferID buffer{ 0 };
			ImageID image;
		};
		uint32_t buffer_offset{ 0 };
		size_t buffer_range{ 0 };
		uint32_t count{ 0 };
		DescriptorType type;
		PipelineShaderStageType shader_stages;
		bool b_supports_bindless{ false };
	};

	struct DescriptorBufferDesc
	{
		void* buffer{ nullptr };
		size_t buffer_size{ 0 };
		size_t buffer_range{ 0 };
		size_t buffer_offset{ 0 };
	};

	struct DescriptorWrite
	{
		uint16_t slot{ 0 };
		uint32_t count{ 0 };
		uint32_t array_index{ 0 };
		DescriptorType type;
		DescriptorBufferDesc buffer_desc;
		class DescriptorSet* set{ nullptr };
	};

	struct DescriptorBindlessWrite
	{
		uint16_t slot{ 0 };
		DescriptorType type;
		void* buffer{ nullptr };
		class DescriptorSet* set{ nullptr };
	};

	struct DescriptorBindlessResourceIndices
	{
		std::vector<int32_t> indices;

		constexpr bool empty() const noexcept
		{
			return indices.empty();
		}
	};

	struct DescriptorBinding
	{
		uint16_t slot{ 0 };
		uint32_t count{ 0 };
		DescriptorType type;
		PipelineShaderStageType pipeline_stages;
		bool b_supports_bindless{ false };

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