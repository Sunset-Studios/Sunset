#include <graphics/descriptor.h>
#include <graphics/asset_pool.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/image.h>

namespace Sunset
{
	Sunset::DescriptorLayoutID DescriptorLayoutCache::fetch_or_add(const std::vector<DescriptorBinding>& bindings, class GraphicsContext* const gfx_context)
	{
		DescriptorLayoutID id{ 0 };
		for (const DescriptorBinding& binding : bindings)
		{
			id = Maths::cantor_pair_hash(static_cast<int32_t>(id), static_cast<int32_t>(std::hash<DescriptorBinding>{}(binding)));
		}
		if (descriptor_layouts.find(id) == descriptor_layouts.end())
		{
			DescriptorLayout* const new_descriptor_layout = GlobalAssetPools<DescriptorLayout>::get()->allocate();
			new_descriptor_layout->set_bindings(bindings);
			new_descriptor_layout->build(gfx_context);
			descriptor_layouts.insert({ id, new_descriptor_layout });
		}
		return id;
	}

	Sunset::DescriptorLayout* DescriptorLayoutCache::fetch(DescriptorLayoutID layout_id)
	{
		assert(descriptor_layouts.find(layout_id) != descriptor_layouts.end());
		return descriptor_layouts[layout_id];
	}

	Sunset::DescriptorSetBuilder DescriptorSetBuilder::begin(class GraphicsContext* const context)
	{
		DescriptorSetBuilder builder;
		builder.gfx_context = context;
		return builder;
	}

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer(uint16_t binding, Buffer* buffer, DescriptorType type, PipelineShaderStageType shader_stages)
	{
		bindings.push_back({ .slot = binding, .count = 1, .type = type, .pipeline_stages = shader_stages });
		descriptor_writes.push_back({ .slot = binding, .count = 1, .type = type, .buffer = buffer->get(), .buffer_size = buffer->get_size(), .set = nullptr });
		return *this;
	}

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer(const DescriptorBuildData& build_data)
	{
		return bind_buffer(build_data.binding, build_data.buffer, build_data.type, build_data.shader_stages);
	}

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_image(uint16_t binding, Image* image, DescriptorType type, PipelineShaderStageType shader_stages)
	{
		bindings.push_back({ .slot = binding, .count = 1, .type = type, .pipeline_stages = shader_stages });
		descriptor_writes.push_back({ .slot = binding, .count = 1, .type = type, .buffer = image->get_image(), .buffer_size = 0, .set = nullptr });
		return *this;
	}

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_image(const DescriptorBuildData& build_data)
	{
		return bind_image(build_data.binding, build_data.image, build_data.type, build_data.shader_stages);
	}

	bool DescriptorSetBuilder::build(DescriptorSet*& out_descriptor_set, DescriptorLayout*& out_descriptor_layout)
	{
		DescriptorLayoutID layout_id = DescriptorLayoutCache::get()->fetch_or_add(bindings, gfx_context);
		out_descriptor_layout = DescriptorLayoutCache::get()->fetch(layout_id);
		
		out_descriptor_set = gfx_context->get_descriptor_set_allocator()->allocate(gfx_context, out_descriptor_layout);
		if (out_descriptor_set == nullptr)
		{
			return false;
		}

		for (DescriptorWrite& write : descriptor_writes)
		{
			write.set = out_descriptor_set;
		}

		gfx_context->push_descriptor_writes(descriptor_writes);

		return true;
	}
}
