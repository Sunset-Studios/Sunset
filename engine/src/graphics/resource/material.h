#pragma once

#include <pipeline_types.h>
#include <descriptor_types.h>
#include <image_types.h>
#include <graphics/resource/resource_cache.h>
#include <gpu_shared_data_types.h>
#include <utility/strings.h>

namespace Sunset
{
	using ImagePathList = std::array<const char*, MAX_MATERIAL_TEXTURES>;

	struct MaterialDescription
	{
		glm::vec3 color{ 1.0f, 1.0f, 1.0f };
		float uniform_roughness{ 1.0f };
		float uniform_metallic{ 0.0f };
		float uniform_reflectance{ 0.0f };
		float uniform_clearcoat{ 0.0f };
		float uniform_clearcoat_roughness{ 1.0f };
		ImagePathList textures;
	};

	struct MaterialData
	{
		glm::vec3 color{ 1.0f, 1.0f, 1.0f };
		float uniform_roughness{ 1.0f };
		float uniform_metallic{ 0.0f };
		float uniform_reflectance{ 0.0f };
		float uniform_clearcoat{ 0.0f };
		float uniform_clearcoat_roughness{ 1.0f };
		int32_t textures[MAX_MATERIAL_TEXTURES];
		float tiling_coeffs[MAX_MATERIAL_TEXTURES];
	};

	DECLARE_GPU_SHARED_DATA(MaterialData, MAX_MATERIALS);

	struct Material
	{
		Material();
		~Material();

		MaterialDescription description;
		std::vector<ImageID> textures;
		std::array<BindingTableHandle, MAX_MATERIAL_TEXTURES> bound_texture_handles;
		MaterialData* gpu_data{ nullptr };
		uint32_t gpu_data_buffer_offset{ 0 };
		bool b_dirty{ true };
		bool b_needs_texture_upload{ false };

		void destroy(class GraphicsContext* const context) { }
	};

	void material_load_textures(class GraphicsContext* const gfx_context, MaterialID material);
	void material_set_gpu_params(class GraphicsContext* const gfx_context, MaterialID material);
	void material_update(class GraphicsContext* const gfx_context, MaterialID material, class DescriptorSet* descriptor_set);
	void material_set_texture_tiling(class GraphicsContext* const gfx_context, MaterialID material, uint32_t texture_index, float texture_tiling);
	void material_set_color(class GraphicsContext* const gfx_context, MaterialID material, glm::vec3 color);
	void material_set_uniform_roughness(class GraphicsContext* const gfx_context, MaterialID material, float roughness);
	void material_set_uniform_metallic(class GraphicsContext* const gfx_context, MaterialID material, float metallic);
	void material_set_uniform_reflectance(class GraphicsContext* const gfx_context, MaterialID material, float reflectance);
	void material_set_uniform_clearcoat(class GraphicsContext* const gfx_context, MaterialID material, float clearcoat);
	void material_set_uniform_clearcoat_roughness(class GraphicsContext* const gfx_context, MaterialID material, float clearcoat_roughness);



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
			if (i != nullptr)
			{
				Sunset::Identity id(i);
				textures_seed ^= id.computed_hash + 0x9e3779b9 + (textures_seed << 6) + (textures_seed >> 2);
			}
		}

		std::size_t final_hash = Sunset::Maths::cantor_pair_hash(static_cast<int32_t>(textures_seed), static_cast<int32_t>(mat.uniform_roughness));

		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(mat.color.r));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(mat.color.g));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(mat.color.b));

		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(mat.uniform_metallic));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(mat.uniform_reflectance));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(mat.uniform_clearcoat));
		final_hash = Sunset::Maths::cantor_pair_hash(final_hash, static_cast<int32_t>(mat.uniform_clearcoat_roughness));

		return final_hash;
	}
};

#pragma warning( pop ) 