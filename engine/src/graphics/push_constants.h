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
