#pragma once

#include <minimal.h>
#include <pipeline_types.h>

namespace Sunset
{
	struct PushConstantPipelineData
	{
		int32_t offset{ 0 };
		size_t size{ 0 };
		PipelineShaderStageType shader_stage{ PipelineShaderStageType::Vertex };
		void* data;

		template<class T>
		static PushConstantPipelineData create(T* type_data)
		{
			PushConstantPipelineData push_constant_data;
			push_constant_data.size = sizeof(T);
			push_constant_data.data = type_data;
			return push_constant_data;
		}
	};
}

#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4267)

template<>
struct std::hash<Sunset::PushConstantPipelineData>
{
	std::size_t operator()(const Sunset::PushConstantPipelineData& pc_data) const
	{
		std::size_t hash = Sunset::Maths::cantor_pair_hash(pc_data.offset, static_cast<int32_t>(pc_data.size));
		hash = Sunset::Maths::cantor_pair_hash(hash, static_cast<int32_t>(pc_data.shader_stage));
		hash = Sunset::Maths::cantor_pair_hash(hash, reinterpret_cast<uintptr_t>(pc_data.data));
		return hash;
	}
};

#pragma warning( pop ) 
