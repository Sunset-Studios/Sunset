#include <graphics/descriptor.h>
#include <graphics/asset_pool.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/image.h>

namespace Sunset
{
	Sunset::DescriptorSetBuilder DescriptorSetBuilder::begin(class GraphicsContext* const context)
	{
		DescriptorSetBuilder builder;
		builder.gfx_context = context;
		return builder;
	}

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer(uint16_t binding, Buffer* buffer, size_t range_size, DescriptorType type, PipelineShaderStageType shader_stages)
	{
		bindings.push_back({ .slot = binding, .count = 1, .type = type, .pipeline_stages = shader_stages });
		descriptor_writes.push_back({ .slot = binding, .count = 1, .type = type, .buffer = buffer->get(), .buffer_size = buffer->get_size(), .buffer_range = range_size, .set = nullptr });
		return *this;
	}

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer(const DescriptorBuildData& build_data)
	{
		return bind_buffer(build_data.binding, build_data.buffer, build_data.buffer_range, build_data.type, build_data.shader_stages);
	}

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_image(uint16_t binding, Image* image, size_t range_size, DescriptorType type, PipelineShaderStageType shader_stages)
	{
		bindings.push_back({ .slot = binding, .count = 1, .type = type, .pipeline_stages = shader_stages });
		descriptor_writes.push_back({ .slot = binding, .count = 1, .type = type, .buffer = image, .buffer_size = 0, .buffer_range = range_size, .set = nullptr });
		return *this;
	}

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_image(const DescriptorBuildData& build_data)
	{
		return bind_image(build_data.binding, build_data.image, build_data.buffer_range, build_data.type, build_data.shader_stages);
	}

	bool DescriptorSetBuilder::build(DescriptorSet*& out_descriptor_set, DescriptorLayout*& out_descriptor_layout)
	{
		Identity cache_id;
		for (const DescriptorBinding& binding : bindings)
		{
			cache_id = Maths::cantor_pair_hash(static_cast<int32_t>(cache_id), static_cast<int32_t>(std::hash<DescriptorBinding>{}(binding)));
		}

		DescriptorLayoutID layout_id = DescriptorLayoutCache::get()->fetch_or_add(cache_id, gfx_context);
		out_descriptor_layout = DescriptorLayoutCache::get()->fetch(layout_id);

		out_descriptor_layout->set_bindings(bindings);
		out_descriptor_layout->build(gfx_context);
		
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

	void DescriptorHelpers::inject_descriptors(GraphicsContext* const context, DescriptorData& out_descriptor_data, const std::vector<DescriptorBuildData>& descriptor_build_datas)
	{
		if (out_descriptor_data.descriptor_set == nullptr)
		{
			DescriptorSetBuilder builder = DescriptorSetBuilder::begin(context);
			for (const DescriptorBuildData& descriptor_build_data : descriptor_build_datas)
			{
				// TODO: Switch on descriptor type to determine whether to bind buffer or image
				if (descriptor_build_data.type == DescriptorType::Image)
				{
					builder.bind_image(descriptor_build_data);
				}
				else
				{
					builder.bind_buffer(descriptor_build_data);
				}

				if (descriptor_build_data.type == DescriptorType::DynamicUniformBuffer)
				{
					out_descriptor_data.dynamic_buffer_offsets.push_back(descriptor_build_data.buffer_offset);
				}
			}
			builder.build(out_descriptor_data.descriptor_set, out_descriptor_data.descriptor_layout);
		}
	}
}
