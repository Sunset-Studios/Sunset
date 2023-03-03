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
		ImagePathList textures;
	};

	struct Material
	{
		MaterialDescription description;
		std::vector<ImageID> textures;
		MaterialData* gpu_data{ nullptr };
		uint32_t gpu_data_buffer_offset{ 0 };
		bool b_dirty{ true };
		bool b_needs_texture_upload{ false };

		void destroy(class GraphicsContext* const context) { }
	};

	void material_load_textures(class GraphicsContext* const gfx_context, MaterialID material);
	void material_upload_textures(class GraphicsContext* const gfx_context, MaterialID material, class DescriptorSet* descriptor_set);
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
		std::size_t textures_seed = mat.textures.size();
		for (auto& i : mat.textures)
		{
			std::size_t hash = reinterpret_cast<uintptr_t>(i);
			textures_seed ^= hash + 0x9e3779b9 + (textures_seed << 6) + (textures_seed >> 2);
		}

		return textures_seed;
	}
};

#pragma warning( pop ) 