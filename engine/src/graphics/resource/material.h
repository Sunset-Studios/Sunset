#pragma once

#include <pipeline_types.h>
#include <descriptor_types.h>
#include <image_types.h>
#include <graphics/resource/resource_cache.h>
#include <gpu_shared_data_types.h>

namespace Sunset
{
	using ImagePathList = std::array<const char*, MAX_MATERIAL_TEXTURES>;

	struct MaterialDescription
	{
		PipelineShaderPathList shaders;
		ImagePathList textures;
	};

	struct Material
	{
		MaterialDescription description;
		std::vector<ShaderID> shaders;
		std::vector<ImageID> textures;
		PipelineStateID pipeline_state;
		DescriptorData descriptor_data;
		MaterialData* gpu_data{ nullptr };
		uint32_t gpu_data_buffer_offset{ 0 };
		bool b_dirty{ true };

		void destroy(class GraphicsContext* const context) { }
	};

	void material_setup_pipeline_state(class GraphicsContext* const gfx_context, MaterialID material, RenderPassID render_pass);
	void material_load_textures(class GraphicsContext* const gfx_context, MaterialID material);
	void material_upload_textures(class GraphicsContext* const gfx_context, MaterialID material, class DescriptorSet* descriptor_set);
	PipelineStateID material_get_pipeline(MaterialID material);
	void material_bind_pipeline(class GraphicsContext* const gfx_context, void* cmd_buffer, MaterialID material);
	void material_bind_descriptors(class GraphicsContext* const gfx_context, void* cmd_buffer, MaterialID material);
	void material_track_gpu_shared_data(class GraphicsContext* const gfx_context, MaterialID material);

	class MaterialFactory
	{
	public:
		static MaterialID create(class GraphicsContext* const gfx_context, const MaterialDescription& desc);
	};

	DEFINE_RESOURCE_CACHE(MaterialCache, MaterialID, Material);
}

#pragma warning( push )
#pragma warning( disable : 4244)
#pragma warning( disable : 4267)

template<>
struct std::hash<Sunset::MaterialDescription>
{
	std::size_t operator()(const Sunset::MaterialDescription& mat) const
	{
		std::size_t shaders_seed = mat.shaders.size();
		for (auto& i : mat.shaders)
		{
			std::size_t hash = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(i.first), reinterpret_cast<uintptr_t>(i.second));
			shaders_seed ^= hash + 0x9e3779b9 + (shaders_seed << 6) + (shaders_seed >> 2);
		}

		std::size_t textures_seed = mat.textures.size();
		for (auto& i : mat.textures)
		{
			std::size_t hash = reinterpret_cast<uintptr_t>(i);
			textures_seed ^= hash + 0x9e3779b9 + (textures_seed << 6) + (textures_seed >> 2);
		}

		std::size_t final_hash = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(shaders_seed), static_cast<int32_t>(textures_seed));

		return final_hash;
	}
};

#pragma warning( pop ) 