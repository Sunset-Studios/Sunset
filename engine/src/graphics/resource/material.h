#pragma once

#include <pipeline_types.h>
#include <descriptor_types.h>
#include <image_types.h>
#include <graphics/resource/resource_cache.h>

namespace Sunset
{
	struct Material
	{
		PipelineShaderPathList shaders;
		ImagePathList textures;
		PipelineStateID pipeline_state;
		DescriptorData descriptor_datas[MAX_BOUND_DESCRIPTOR_SETS];
	};

	void bind_material_pipeline(class GraphicsContext* const gfx_context, void* cmd_buffer, MaterialID material);
	PipelineStateID get_material_pipeline(MaterialID material);
	void bind_material_descriptors(class GraphicsContext* const gfx_context, void* cmd_buffer, MaterialID material);

	DEFINE_RESOURCE_CACHE(MaterialCache, MaterialID, Material);
}

#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4267)

template<>
struct std::hash<Sunset::Material>
{
	std::size_t operator()(const Sunset::Material& psd) const
	{
		std::size_t shaders_seed = psd.shaders.size();
		for (auto& i : psd.shaders)
		{
			std::size_t hash = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(i.first), reinterpret_cast<uintptr_t>(i.second));
			shaders_seed ^= hash + 0x9e3779b9 + (shaders_seed << 6) + (shaders_seed >> 2);
		}

		std::size_t textures_seed = psd.textures.size();
		for (auto& i : psd.textures)
		{
			std::size_t hash = reinterpret_cast<uintptr_t>(i);
			textures_seed ^= hash + 0x9e3779b9 + (textures_seed << 6) + (textures_seed >> 2);
		}

		std::size_t ps_seed = psd.pipeline_state;
		
		std::size_t ds_seed = Sunset::MAX_BOUND_DESCRIPTOR_SETS;
		for (int i = 0; i < Sunset::MAX_BOUND_DESCRIPTOR_SETS; ++i)
		{
			std::size_t hash = Sunset::Maths::cantor_pair_hash(reinterpret_cast<uintptr_t>(psd.descriptor_datas[i].descriptor_layout), reinterpret_cast<uintptr_t>(psd.descriptor_datas[i].descriptor_set));
			ds_seed ^= hash + 0x9e3779b9 + (ds_seed << 6) + (ds_seed >> 2);
		}

		std::size_t final_hash = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(shaders_seed), static_cast<int32_t>(textures_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(ps_seed));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(ds_seed));

		return final_hash;
	}
};

#pragma warning( pop ) 