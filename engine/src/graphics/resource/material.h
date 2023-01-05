#pragma once

#include <pipeline_types.h>
#include <descriptor_types.h>
#include <image_types.h>

namespace Sunset
{
	struct Material
	{
		PipelineShaderPathList shaders;
		ImagePathList textures;
		PipelineStateID pipeline_state;
		DescriptorData descriptor_data;
	};
}
